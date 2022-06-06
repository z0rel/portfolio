-- Function: edit_user_cert_roles(bigint, role_group, varchar[], timestamptz)

DROP FUNCTION edit_user_cert_roles(bigint, role_group, varchar[], timestamptz);

CREATE OR REPLACE FUNCTION edit_user_cert_roles(
		IN cert_id_in bigint,
		IN roles_type_in role_group,
		IN new_roles_str_id_in varchar[],
		IN last_permission_modify_in timestamptz,

		OUT cert_subject_cn_out varchar,
		OUT added_roles_out varchar[],
		OUT deleted_roles_out varchar[])
	AS
$BODY$
declare
	id_role_ bigint;
	subject_id_ bigint;
	user_perm_last_modify_date_ timestamptz;
	added_roles_ bigint[];
	deleted_roles_ bigint[];
	resource_id_ bigint;

	rid_ int;
	iid_ int;
	serial_number_ varchar;
	user_cert_issuer_ varchar;
	user_cert_akeyid_ bytea;
	need_update_version_ boolean;

	max_users_ bigint;
	current_users_ bigint;
begin
	if cert_id_in is null or roles_type_in is null
	then
		execute 'select add_last_error(-6);'; --NULL недопустим
		return;
	end if;

	select UC.serial_number, UC.issuer_dn, UC.issuer_key_identifier, UC.last_permissions_modify_date, UC.subject_cn
	from users_certificates UC
	where UC.cert_id = cert_id_in
	into serial_number_, user_cert_issuer_, user_cert_akeyid_, user_perm_last_modify_date_, cert_subject_cn_out;

	if (user_perm_last_modify_date_ is null)
	then
		execute 'select add_last_error(-8);'; -- Объекта с данным идентификатором нет.
		return;
	end if;

	-- список идентификаторов новых ролей пользователя для указанной группы
	select array(select R.role_id from roles R
								where R.string_id = any (new_roles_str_id_in) and
											R.group_type = roles_type_in
							 EXCEPT
							 select URE.role_id from user_roles_entries URE
								inner join roles R on URE.role_id = R.role_id
								where URE.user_cert_id = cert_id_in and
											R.group_type = roles_type_in)
	into added_roles_;

	select array(select R.string_id
	from roles R
	where R.role_id = any (added_roles_))
	into added_roles_out;

	-- список идентификаторов удаляемых ролей пользователя для указанной группы
	select array( select URE.role_id from user_roles_entries URE
								inner join roles R on URE.role_id = R.role_id
								where URE.user_cert_id = cert_id_in and
											R.group_type = roles_type_in
								EXCEPT
								select R.role_id from roles R
								where R.string_id = any (new_roles_str_id_in) and
											R.group_type = roles_type_in )
	into deleted_roles_;

	select array(select R.string_id
	from roles R
	where R.role_id = any (deleted_roles_))
	into deleted_roles_out;

	-- Получаем ID текущего сертификата пользователя
	select into subject_id_ get_current_user_cert_id();

	-- проверяем разрешение на изменение прав сертификата
	if not check_permission_to_object(subject_id_, cert_id_in, roles_type_in, 'w')
	then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	if (last_permission_modify_in != user_perm_last_modify_date_)
	then
		execute 'select add_last_error(-20);'; -- Редактирование объекта невозможно, состояние было изменено.
		return;
	end if;

	need_update_version_:=false;

	-- добавляем роли, если их нет у сертификата
	foreach id_role_ in array added_roles_
	loop
		insert into user_roles_entries(user_cert_id, role_id) values(cert_id_in, id_role_);

		-- если устанавливается метка блокированного пользователя удаляем и права на ресурсы
		if (id_role_ = (select role_id from roles R where R.string_id = 'bu'))
		then
			--nginxdb_proxy
			delete from nginxdb_proxy_permissions NDBP where NDBP.cert_serial_number = serial_number_;
			need_update_version_:=true;
		elseif (id_role_ = (select role_id from roles R where R.string_id = 'u'))
		then

			select TGO.intvalue from tlsgtw_options TGO where TGO.nameoption = 'max_users' into max_users_;
			select count(*) from user_roles_entries URE
			inner join roles R on R.role_id = URE.role_id
			where R.string_id = 'u' into current_users_;

			if ((select TGO.intvalue from tlsgtw_options TGO where TGO.nameoption = 'is_present') = 0) or max_users_ is null
			then
				-- возврат ошибки без отката изменений возможен, так админские роли редактируются отдельной группой
				execute 'select add_last_error(-32);'; -- Лицензия не загружена
				return;
			end if;

			-- если пользователь новый (добавляется разрешение или запрет по крайней мере на 1 ресурс - добавляется роль пользователя)
			-- нужно проверить превышение лицензионных ограничений
			if (current_users_ >= max_users_)
			then
				-- возврат ошибки без отката изменений возможен, так админские роли редактируются отдельной группой
				execute 'select add_last_error(-33);'; -- Достигнуто максимальное число пользователей для данной лицензии
				return;
			end if;
		end if;

	end loop;

		-- удаляем роли из существующих, если они не входят в новый список
	foreach id_role_ in array deleted_roles_
	loop
		if (id_role_ = (select role_id from roles R where R.string_id = 'seca'))
		then
			if ((select count(*) from user_roles_entries URE where URE.role_id = id_role_) = 1)
			then
				execute 'select add_last_error(-23);'; -- невозможно удалить последнего администратора безопасности
				continue;
			end if;
		end if;

		delete from user_roles_entries URE where URE.user_cert_id = cert_id_in and URE.role_id = id_role_;

		-- восстанавливаем разрешения для nginxdb_proxy из user_resources_access_entries
		-- если удаляется метка блокированного пользователя и сертификат не отозван и не отозван сертификат доверенного УЦ, выпустившего сертификат
		if (id_role_ = (select role_id from roles R where R.string_id = 'bu'))
						and not exists (select * from revoked_certs RC
												where
												(RC.serial_number = serial_number_ and RC.trusted_root_cert_id = (
														select TRC.root_cert_id
														from trusted_root_certificate TRC
					 									where TRC.subject_dn = user_cert_issuer_ and
					 												TRC.subject_key_identifier = user_cert_akeyid_))
												or
												(RC.serial_number = (
														select TRC.cert_serial
														from trusted_root_certificate TRC
														where TRC.subject_dn = user_cert_issuer_ and
																	TRC.subject_key_identifier = user_cert_akeyid_)
												and RC.trusted_root_cert_id = (
														select TRC.root_cert_id
														from trusted_root_certificate TRC
														where TRC.subject_dn = user_cert_issuer_ and
														TRC.subject_key_identifier = user_cert_akeyid_)))
		then
			for resource_id_ in (select URAE.resource_id from user_resources_access_entries URAE where URAE.user_cert_id = cert_id_in)
			loop
				if (select URAE.access_allowed from user_resources_access_entries URAE where URAE.user_cert_id = cert_id_in and URAE.resource_id = resource_id_)
				then

					select into rid_ NDBR.rid
					from nginxdb_proxy_resources NDBR
					where NDBR.external_addr = (select PR.external_address from proxy_resources PR where PR.resource_id = resource_id_);

					select into iid_ NDBI.iid
					from nginxdb_proxy_issuer NDBI
					where NDBI.issuer_dn = user_cert_issuer_;

					if not (rid_ is null) and not (iid_ is null) and not (serial_number_ is null)
					then
						insert into nginxdb_proxy_permissions (iid, rid, cert_serial_number) values (iid_,rid_,serial_number_);
					end if;

				end if;
			end loop;

			need_update_version_:=true;
		end if;

		if (id_role_ = (select role_id from roles R where R.string_id = 'u'))
		then
			delete from user_resources_access_entries URAE where URAE.user_cert_id = cert_id_in;

			-- nginxdb_proxy
			delete from nginxdb_proxy_permissions NDBP where NDBP.cert_serial_number = serial_number_;

			need_update_version_:=true;
		end if;
	end loop;

	if (need_update_version_)
	then
		-- меняем версию БД
		update db_version set dbversion = dbversion + 1;
	end if;

	-- обновляем дату изменения разрешений сертификата
	update users_certificates set last_permissions_modify_date = now()
	where cert_id = cert_id_in;
end;
$BODY$
	LANGUAGE plpgsql VOLATILE
	COST 100;
ALTER FUNCTION edit_user_cert_roles(bigint, role_group, varchar[], timestamptz)
	OWNER TO postgres;
COMMENT ON FUNCTION edit_user_cert_roles(bigint, role_group, varchar[], timestamptz) IS 'Редактирует роли для сертификата пользователя.
Все имеющиеся роли типа roles_type, не вошедшие в список new_roles_str_id_in, будут удалены.';
