-- Function: edit_proxy_resource(bigint, timestamptz, varchar, varchar, varchar, varchar, tls_type, varchar)

DROP FUNCTION  edit_proxy_resource(bigint, timestamptz, varchar, varchar, varchar, varchar, tls_type, varchar);

CREATE OR REPLACE FUNCTION edit_proxy_resource(
		IN id_in bigint,
		IN last_modify_date_in timestamptz,

		IN new_name_in varchar,
		IN new_description_in varchar,
		IN new_internal_address_in varchar,
		IN new_external_address_in varchar,
		IN new_auth_mode_in tls_type,
		IN new_subfilters_in varchar,

		OUT old_name_out varchar,
		OUT old_description_out varchar,
		OUT old_internal_address_out varchar,
		OUT old_external_address_out varchar,
		OUT old_auth_mode_out tls_type,
		OUT old_subfilters_out varchar,
		OUT new_last_modify_date_out timestamptz) AS
$BODY$
declare
	subject_id_ bigint;
	last_modify_date_ timestamptz;
	user_id_ bigint;

	rid_ int;
	iid_ int;
	serial_number_ varchar;
	user_cert_issuer_ varchar;
	user_cert_akeyid_ bytea;
begin
	if (id_in is null or last_modify_date_in is null) then
		execute 'select add_last_error(-6);'; --NULL недопустим
		return;
	end if;

	-- Получаем ID текущего сертификата пользователя
	select into subject_id_ get_current_user_cert_id();

	-- Дата последнего изменения параметров проксируемого ресурса
	select into last_modify_date_ last_modify_date from proxy_resources PR where resource_id = id_in;

	if (last_modify_date_ is null)
	then
		execute 'select add_last_error(-8);'; --Объекта с данным идентификатором нет
		return;
	end if;

	if (last_modify_date_in != last_modify_date_)
	then
		execute 'select add_last_error(-20);'; -- Редактирование объекта невозможно, состояние было изменено.
		return;
	end if;

	-- получает текущие параметры
	select PR.resource_name, PR.resource_description, PR.internal_address, PR.external_address, PR.auth_mode, PR.nginxdb_proxy_subfilter
	from proxy_resources PR
	where PR.resource_id = id_in
	into old_name_out, old_description_out, old_internal_address_out, old_external_address_out, old_auth_mode_out, old_subfilters_out;

	if not check_permission_to_object(subject_id_, id_in, null, 'w')
	then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	if not exists (select resource_name from proxy_resources where resource_id = id_in and resource_name = new_name_in) -- если это не имя текущего id ресурса
		and exists (select resource_name from proxy_resources where resource_name = new_name_in)
	then
		execute 'select add_last_error(-17);'; --Ресурс с заданным именем уже существует
		return;
	end if;

	if not exists (select internal_address from proxy_resources where resource_id = id_in and internal_address = new_internal_address_in) -- если это не внешний адрес текущего id ресурса
			and exists (select internal_address from proxy_resources where internal_address = new_internal_address_in)
	then
		execute 'select add_last_error(-18);'; --Внутренний адрес ресурса уже используется
		return;
	end if;

	if not exists (select external_address from proxy_resources where resource_id = id_in and external_address = new_external_address_in) -- если это не внутренний адрес текущего id ресурса
			and exists (select external_address from proxy_resources where external_address = new_external_address_in)
	then
		execute 'select add_last_error(-19);'; --Внешний адрес ресурса уже используется
		return;
	end if;

	-- меняем на новые
	update proxy_resources set
		resource_name = coalesce(new_name_in, resource_name),
		resource_description = coalesce(new_description_in, resource_description),
		internal_address = coalesce(new_internal_address_in, internal_address),
		external_address = coalesce(new_external_address_in, external_address),
		last_modify_date = now(),
		auth_mode = coalesce(new_auth_mode_in, auth_mode),
		nginxdb_proxy_subfilter = coalesce(new_subfilters_in, nginxdb_proxy_subfilter)
	where resource_id = id_in returning last_modify_date into new_last_modify_date_out;

	-- nginxdb_proxy
	if (new_auth_mode_in = 'oneSideTLS')
	then
		delete from nginxdb_proxy_resources NDBR where NDBR.external_addr = old_external_address_out returning NDBR.rid into rid_;
		delete from nginxdb_proxy_permissions NDBP where NDBP.rid = rid_;

		-- меняем версию БД
		update db_version set dbversion = dbversion + 1;

	elseif  (new_auth_mode_in = 'twoSideTLS')
	then
		if exists (select * from nginxdb_proxy_resources NDBR where NDBR.external_addr = old_external_address_out)
		then
			update nginxdb_proxy_resources NDBR
			set NDBR.external_addr = coalesce(new_external_address_in, NDBR.external_addr)
			where NDBR.external_addr = old_external_address_out;
		else
			insert into nginxdb_proxy_resources (external_addr) values (coalesce(new_external_address_in, old_external_address_out));
		end if;

		-- был односторонний TLS стал двусторонний - нужно выгрузить права из access entries в nginxdb_proxy_permissions, если они есть
		if (old_auth_mode_out = 'oneSideTLS')
		then
			for user_id_ in (select URAE.user_cert_id from user_resources_access_entries URAE where URAE.resource_id = id_in)
			loop
				select UC.serial_number, UC.issuer_dn, UC.issuer_key_identifier
				from users_certificates UC
				where UC.cert_id = user_id_
				into serial_number_, user_cert_issuer_, user_cert_akeyid_;

				-- если есть разрешение и сертификат не отозван и не отозван корневой - добавляем в таблицу разрешений для nginxdb_proxy
				if (select URAE.access_allowed from user_resources_access_entries URAE where URAE.user_cert_id = user_id_ and URAE.resource_id = id_in)
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
					select into rid_ NDBR.rid
					from nginxdb_proxy_resources NDBR
					where NDBR.external_addr = (select PR.external_address from proxy_resources PR where PR.resource_id = id_in);

					select into iid_ NDBI.iid
					from nginxdb_proxy_issuer NDBI
					where NDBI.issuer_dn = user_cert_issuer_;

					if not (rid_ is null) and not (iid_ is null) and not (serial_number_ is null)
					then
						insert into nginxdb_proxy_permissions (iid, rid, cert_serial_number) values (iid_,rid_,serial_number_);
					end if;

				end if;
			end loop;

			-- меняем версию БД
			update db_version set dbversion = dbversion + 1;
		end if;
	end if;

end;
$BODY$
	LANGUAGE plpgsql VOLATILE
	COST 100;
ALTER FUNCTION edit_proxy_resource(bigint, timestamptz, varchar, varchar, varchar, varchar, tls_type, varchar)
	OWNER TO postgres;
COMMENT ON FUNCTION edit_proxy_resource(bigint, timestamptz, varchar, varchar, varchar, varchar, tls_type, varchar) IS 'Редактирует информацию о проксируемом ресурсе';
