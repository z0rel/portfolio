-- Function: list_user_requests(varchar, bigint[])

DROP FUNCTION list_user_requests(varchar, bigint[]);

CREATE OR REPLACE FUNCTION list_user_requests(search_subject_name varchar, resources_filter bigint[])
returns table (
	cert_id bigint,
  serial_number varchar,
	subject_cn varchar(64),
  subject_o varchar(1024),
	subject_snils character(11),
	issuer_cn varchar(64),
	valid_from timestamp with time zone,
	valid_to timestamp with time zone,
	last_permissions_modify timestamptz,
	is_valid boolean,

	request_date timestamptz,
	requested_resources_names varchar[],
	request_description varchar,
	user_email varchar) as
$BODY$
declare
	subject_id_ bigint;
	user_cert_list_filter_ text;
	list_users_requests_query_ text;
begin
	-- Получаем ID текущего сертификата пользователя
	select into subject_id_ get_current_user_cert_id();

	if not check_permission_to_object_type(subject_id_, 'certificate', null, 'r') then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	user_cert_list_filter_:='';

	if not (resources_filter is null) and ( array_length(resources_filter, 1) >= 1 ) then
		user_cert_list_filter_ := user_cert_list_filter_ ||
		'(array(select RRE.requested_resource_id from resources_requests_entries RRE where RRE.user_cert_id = UC.cert_id) &&
		ARRAY[' || array_to_string(resources_filter, ',', 'null') || ']::bigint[])';
	end if;

	if not (search_subject_name is null) then
		if (char_length(user_cert_list_filter_) > 0) then
			user_cert_list_filter_ := user_cert_list_filter_ || ' and ';
		end if;
		user_cert_list_filter_ := user_cert_list_filter_ || 'UC.subject_cn ~* concat_ws(''.*'', ' || quote_nullable(search_subject_name) || ' , ''.*'')';
	end if;

	if (char_length(user_cert_list_filter_) = 0) then
	    user_cert_list_filter_ := ' true';
	end if;

	list_users_requests_query_ := 'select
				UC.cert_id,
				UC.serial_number,
				UC.subject_cn,
				UC.subject_o,
				UC.subject_snils,
				UC.issuer_cn,
				cast(UC.valid_from at time zone ''utc'' as timestamptz),
				cast(UC.valid_to at time zone ''utc'' as timestamptz),
				cast(UC.last_permissions_modify_date at time zone ''utc'' as timestamptz),
				not exists(select * from revoked_certs RC
						where RC.trusted_root_cert_id = (select TRC.root_cert_id from trusted_root_certificate TRC
												where TRC.subject_dn = UC.issuer_dn and TRC.subject_key_identifier = UC.issuer_key_identifier) and
									RC.serial_number = UC.serial_number),
				cast(RAR.request_date at time zone ''utc'' as timestamptz),
				array(select PR.resource_name from proxy_resources PR where PR.resource_id = any (array(select RRE.requested_resource_id from resources_requests_entries RRE where RRE.user_cert_id = UC.cert_id))),
				RAR.request_description,
				RAR.user_email
			from users_certificates UC
			inner join resources_access_requests RAR on UC.cert_id = RAR.user_cert_id
			where ' || user_cert_list_filter_ || ' and
						not exists (select * from user_roles_entries URE
												where URE.user_cert_id = UC.cert_id and
															URE.role_id = (select R.role_id from roles R where R.string_id = ''bu'')) and
						exists (select * from resources_requests_entries RRE where RRE.user_cert_id = UC.cert_id)
			order by UC.subject_cn asc limit 10000';

	return query execute list_users_requests_query_;

end;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION list_user_requests(varchar, bigint[])
  OWNER TO postgres;
COMMENT ON FUNCTION list_user_requests(varchar, bigint[]) IS 'Получение запроса на ресурсы текущего пользователя.';
