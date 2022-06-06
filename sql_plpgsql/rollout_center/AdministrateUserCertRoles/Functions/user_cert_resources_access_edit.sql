-- Function: user_cert_resources_access_edit(bigint, bigint[], bigint[], timestamptz)

DROP FUNCTION user_cert_resources_access_edit(bigint, bigint[], bigint[], timestamptz);

CREATE OR REPLACE FUNCTION user_cert_resources_access_edit(
	IN user_cert_id_in bigint,
	IN allowed_resources_ids_in bigint[],
	IN forbidden_resources_ids_in bigint[],
	IN last_permission_modify_in timestamptz,

	OUT usercert_subject_cn_out varchar,
	OUT new_perm_modify_date_out timestamptz,
	OUT allowed_resources_names_out varchar[],
	OUT forbidden_resources_names_out varchar[]
) AS
$BODY$
declare
	subject_id_ bigint;
	last_modify_date_ timestamptz;
	resource_id_ bigint;
	user_requested_resources bigint[];

	rid_ int;
	iid_ int;
	serial_number_ varchar;
	user_cert_issuer_ varchar;
	user_cert_akeyid_ bytea;

	max_users_ bigint;
	current_users_ bigint;
begin
	if user_cert_id_in is null then
		execute 'select add_last_error(-6);'; --NULL недопустим
		return;
	end if;

	if (allowed_resources_ids_in && forbidden_resources_ids_in) then
		execute 'select add_last_error(-3);'; -- Передаваемые аргументы взаимоисключающие.
		return;
	end if;

	select UC.serial_number, UC.issuer_dn, UC.subject_cn, UC.last_permissions_modify_date
	from users_certificates UC
	where UC.cert_id = user_cert_id_in
	into serial_number_, user_cert_issuer_, usercert_subject_cn_out, last_modify_date_;

	if (usercert_subject_cn_out is null)
	then
		execute 'select add_last_error(-8);'; --Объекта с данным идентификатором нет.
		return;
	end if;

	if exists (select * from user_roles_entries URE
		inner join roles R on URE.role_id = R.role_id
		where R.string_id = 'bu' and URE.user_cert_id = user_cert_id_in)
	then
		execute 'select add_last_error(-24);'; --Невозможно редактировать доступ к ресурсам у заблокированного сертификата
		return;
	end if;

	select array(select PR.resource_name
	from proxy_resources PR
	where PR.resource_id = any (allowed_resources_ids_in))
	into allowed_resources_names_out;

	select array(select PR.resource_name
	from proxy_resources PR
	where PR.resource_id = any (forbidden_resources_ids_in))
	into forbidden_resources_names_out;

	if (last_permission_modify_in != last_modify_date_)
	then
		execute 'select add_last_error(-20);'; -- Редактирование объекта невозможно, состояние было изменено.
		return;
	end if;

	-- Получаем ID текущего пользователя
	select into subject_id_ get_current_user_cert_id();

	if not check_permission_to_object(subject_id_, user_cert_id_in, 'user', 'w') or		-- проверка прав на работу с сертификатами
		not check_permission_to_object_type(subject_id_, 'proxy_resource', null, 'w')		-- проверка прав на работу с ресурсами
	then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	select TGO.intvalue from tlsgtw_options TGO where TGO.nameoption = 'max_users' into max_users_;
	select count(*) from user_roles_entries URE
	inner join roles R on R.role_id = URE.role_id
	where R.string_id = 'u' into current_users_;

	if ((select TGO.intvalue from tlsgtw_options TGO where TGO.nameoption = 'is_present') = 0) or max_users_ is null
	then
		execute 'select add_last_error(-32);'; -- Лицензия не загружена
		return;
	end if;

	-- если пользователь новый (добавляется разрешение или запрет по крайней мере на 1 ресурс - добавляется роль пользователя)
	-- нужно проверить превышение лицензионных ограничений
	if not exists(select * from user_roles_entries URE inner join roles R on URE.role_id = R.role_id
								where URE.user_cert_id = user_cert_id_in and R.string_id = 'u') and
			(not (allowed_resources_ids_in is null) or not (forbidden_resources_ids_in is null)) and
			current_users_ >= max_users_
	then
		execute 'select add_last_error(-33);'; -- Достигнуто максимальное число пользователей для данной лицензии
		return;
	end if;

	if not (allowed_resources_ids_in is null)
	then
		foreach resource_id_ in array allowed_resources_ids_in
		loop
			-- добавляем права если ресурс с данным идентификатором существует
			if exists(select * from proxy_resources PR where PR.resource_id = resource_id_)
			then
				-- edit access entries
				if not exists(select * from user_resources_access_entries URAE
										where URAE.user_cert_id = user_cert_id_in and URAE.resource_id = resource_id_)
				then
					insert into user_resources_access_entries(user_cert_id, resource_id, access_allowed)
					values(user_cert_id_in, resource_id_, true);
				else
					update user_resources_access_entries URAE set access_allowed = true
					where resource_id = resource_id_ and user_cert_id = user_cert_id_in;
				end if;

				-- edit user requests
				delete from resources_requests_entries RRE where RRE.user_cert_id = user_cert_id_in and RRE.requested_resource_id = resource_id_;

				-- если сертификат не отозван и не отозван сертификат доверенного УЦ - добавляем в таблицу разрешений для nginxdb_proxy
				if not exists (select * from revoked_certs RC
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
					select into rid_ NDBR.rid
					from nginxdb_proxy_resources NDBR
					where NDBR.external_addr = (select PR.external_address from proxy_resources PR where PR.resource_id = resource_id_);

					select into iid_ NDBI.iid
					from nginxdb_proxy_issuer NDBI
					where NDBI.issuer_dn = user_cert_issuer_;

					if not (rid_ is null) and
						 not (iid_ is null) and
						 not (serial_number_ is null) and
						 not exists(select * from nginxdb_proxy_permissions NDBP where
												NDBP.rid = rid_ and
												NDBP.iid = iid_ and
												NDBP.cert_serial_number = serial_number_)
					then
							insert into nginxdb_proxy_permissions (iid, rid, cert_serial_number)  values (iid_, rid_, serial_number_);
					end if;

				end if;
			end if;

		end loop;
	end if;

	if not (forbidden_resources_ids_in is null)
	then
		foreach resource_id_ in array forbidden_resources_ids_in
		loop
			-- удаляем права если ресурс с данным идентификатором существует
			if exists(select * from proxy_resources PR where PR.resource_id = resource_id_)
			then
				-- edit access entries
				if not exists(select * from user_resources_access_entries URAE
										where URAE.user_cert_id = user_cert_id_in and URAE.resource_id = resource_id_)
				then
					insert into user_resources_access_entries(user_cert_id, resource_id, access_allowed)
					values(user_cert_id_in, resource_id_, false);
				else
					update user_resources_access_entries URAE set access_allowed = false
					where resource_id = resource_id_ and user_cert_id = user_cert_id_in;
				end if;

				-- edit user requests
				delete from resources_requests_entries RRE where RRE.user_cert_id = user_cert_id_in and RRE.requested_resource_id = resource_id_;

				-- nginxdb_proxy
				select into rid_ NDBR.rid
				from nginxdb_proxy_resources NDBR
				where NDBR.external_addr = (select PR.external_address from proxy_resources PR where PR.resource_id = resource_id_);

				select into iid_ NDBI.iid
				from nginxdb_proxy_issuer NDBI
				where NDBI.issuer_dn = user_cert_issuer_;

				if not (rid_ is null) and
					 not (iid_ is null) and
					 not (serial_number_ is null) and
					 exists(select * from nginxdb_proxy_permissions NDBP where
											NDBP.rid = rid_ and
											NDBP.iid = iid_ and
											NDBP.cert_serial_number = serial_number_)
				then
						delete from nginxdb_proxy_permissions NDBP where NDBP.iid = iid_ and NDBP.rid = rid_  and NDBP.cert_serial_number = serial_number_;
				end if;
			end if;
		end loop;
	end if;

	if exists (select * from user_resources_access_entries URAE where URAE.user_cert_id = user_cert_id_in) and
			not exists(select * from user_roles_entries URE inner join roles R on URE.role_id = R.role_id
													where URE.user_cert_id = user_cert_id_in and R.string_id = 'u')
	then
		insert into user_roles_entries(user_cert_id, role_id)
		values(user_cert_id_in, (select R.role_id from roles R where R.string_id = 'u'));
	end if;

	-- обновляем дату изменения разрешений сертификата
	update users_certificates set last_permissions_modify_date = now()
	where cert_id = user_cert_id_in returning cast(last_permissions_modify_date at time zone 'utc' as timestamptz) into new_perm_modify_date_out;

	-- меняем версию БД
	update db_version set dbversion = dbversion + 1;
end;
$BODY$
	LANGUAGE plpgsql VOLATILE
	COST 100;
ALTER FUNCTION user_cert_resources_access_edit(bigint, bigint[], bigint[], timestamptz)
	OWNER TO postgres;
COMMENT ON FUNCTION user_cert_resources_access_edit(bigint, bigint[], bigint[], timestamptz) IS 'Управление доступом пользователя к ресурсам';
