-- function list_user_certificates(varchar, boolean, varchar[])

drop function list_user_certificates(varchar, boolean, varchar[]);

create or replace function list_user_certificates(search_name_in varchar, valid_only_filter_in boolean, roles_filter_in varchar[])
returns table (
	cert_id bigint,
	serial_number varchar,
	subject_cn varchar(64),
	subject_o varchar(1024),
	subject_snils character(11),
	issuer_cn varchar(64),
	valid_from timestamp with time zone,
	valid_to timestamp with time zone,
	cert_roles_ids varchar[],
	commentary varchar,
	is_valid bool,
	last_permissions_modify timestamptz) as
$$
declare
	subject_id_ bigint;
	time_now_ timestamp with time zone;
	roles_filter_ids_ bigint[];
	blocked_user_role_id_ bigint;
	user_cert_list_filter_ text;
begin
	-- Получаем ID текущего сертификата пользователя
	select into subject_id_ get_current_user_cert_id();

	if not check_permission_to_object_type(subject_id_, 'certificate', null, 'r') then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	select into time_now_ now();
	user_cert_list_filter_:='';
	if not (valid_only_filter_in is null) then
		user_cert_list_filter_ := user_cert_list_filter_ || ' UC.valid_from < ' || time_now_ || ' and UC.valid_to > ' || time_now_ || '';
	end if;

	-- список идентификаторов ролей
	select array(select R.role_id from roles R where R.string_id = any (roles_filter_in)) into roles_filter_ids_;
	-- идентификатор роли блокированного пользователя
	select R.role_id from roles R where R.string_id = 'bu' into blocked_user_role_id_;

	if not (roles_filter_in is null) and ( array_length(roles_filter_ids_, 1) >= 1 ) then
		if (char_length(user_cert_list_filter_) > 0) then
			user_cert_list_filter_ := user_cert_list_filter_ || ' and ';
		end if;

			if (roles_filter_in && array['u']::varchar[]) and not (roles_filter_in && array['bu']::varchar[]) then
					user_cert_list_filter_ := user_cert_list_filter_ ||
					'array(select R.role_id from user_roles_entries URE inner join roles R on R.role_id = URE.role_id where URE.user_cert_id = UC.cert_id)
					 && ARRAY[' || array_to_string(roles_filter_ids_, ',', 'null') || ']::bigint[] and not
					 array(select R.role_id from user_roles_entries URE inner join roles R on R.role_id = URE.role_id where URE.user_cert_id = UC.cert_id)
					 && ARRAY[' || blocked_user_role_id_::varchar || ']::bigint[]';
			else
					user_cert_list_filter_ := user_cert_list_filter_ ||
					'array(select R.role_id from user_roles_entries URE inner join roles R on R.role_id = URE.role_id where URE.user_cert_id = UC.cert_id)
					 && ARRAY[' || array_to_string(roles_filter_ids_, ',', 'null') || ']::bigint[]';
			end if;
	end if;

	if not (search_name_in is null) then
		if (char_length(user_cert_list_filter_) > 0) then
			user_cert_list_filter_ := user_cert_list_filter_ || ' and ';
		end if;
		user_cert_list_filter_ := user_cert_list_filter_ ||
			'(UC.subject_cn ~* concat_ws(''.*'', ' || quote_nullable(search_name_in) || ' , ''.*'') or
			UC.subject_o ~* concat_ws(''.*'', ' || quote_nullable(search_name_in) || ' , ''.*'') or
			UC.subject_snils ~* concat_ws(''.*'', ' || quote_nullable(search_name_in) || ' , ''.*''))';
	end if;

	if (char_length(user_cert_list_filter_) = 0) then
			user_cert_list_filter_ := ' true';
	end if;

	return query execute 'select
				UC.cert_id,
				UC.serial_number,
				UC.subject_cn,
				UC.subject_o,
				UC.subject_snils,
				UC.issuer_cn,
				cast(UC.valid_from at time zone ''utc'' as timestamptz),
				cast(UC.valid_to at time zone ''utc'' as timestamptz),
				array(select R.string_id
							from user_roles_entries URE
							full join roles R on R.role_id = URE.role_id
							where URE.user_cert_id = UC.cert_id) as cert_roles_ids,
				UC.commentary,
				not exists(select * from revoked_certs RC
										where RC.trusted_root_cert_id = (select TRC.root_cert_id from trusted_root_certificate TRC
																where TRC.subject_dn = UC.issuer_dn and TRC.subject_key_identifier = UC.issuer_key_identifier) and
														RC.serial_number = UC.serial_number),
				cast(UC.last_permissions_modify_date at time zone ''utc'' as timestamptz)
			from users_certificates UC
			where ' || user_cert_list_filter_ || '
			order by UC.subject_cn asc limit 10000';

end;
$$
language plpgsql;