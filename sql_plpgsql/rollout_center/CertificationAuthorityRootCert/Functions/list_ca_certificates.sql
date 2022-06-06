-- function list_ca_certificates(varchar, bigint)

drop function list_ca_certificates(varchar, bigint);

create or replace function list_ca_certificates(search_name_in varchar, max_count_in bigint default 10000)
returns table (
	root_cert_id bigint,
	serial_number varchar,
	subject_cn varchar(64),
	subject_o varchar(1024),
	issuer_cn varchar(64),
	valid_from timestamp with time zone,
	valid_to timestamp with time zone,
	is_valid boolean,
	last_modify timestamptz,
	crl_data varchar,
	crl_when_created timestamptz,
	crl_when_next timestamptz,
	crl_update_period bigint,
	crl_update_uri varchar) as
$$
declare
	subject_id_ bigint;
	root_cert_list_filter_ text;
begin
	-- Получаем ID текущего сертификата пользователя
	select into subject_id_ get_current_user_cert_id();

	if not check_permission_to_object_type(subject_id_, 'ca_certificate', null, 'r') then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	root_cert_list_filter_:='';

	if not (search_name_in is null) then
		root_cert_list_filter_ := root_cert_list_filter_ ||
			'(TRC.subject_cn ~* concat_ws(''.*'', ' || quote_nullable(search_name_in) || ' , ''.*'') or
			TRC.subject_o ~* concat_ws(''.*'', ' || quote_nullable(search_name_in) || ' , ''.*''))';
	end if;

	if (char_length(root_cert_list_filter_) = 0) then
			root_cert_list_filter_ := ' true';
	end if;

	-- as_valid false - отозван корневой доверенный сертификат
	return query execute 'select
		TRC.root_cert_id,
		TRC.cert_serial,
		TRC.subject_cn,
		TRC.subject_o,
		TRC.issuer_cn,
		cast(TRC.valid_from at time zone ''utc'' as timestamptz),
		cast(TRC.valid_to at time zone ''utc'' as timestamptz),
		not exists(select * from revoked_certs RC
								where RC.trusted_root_cert_id = (select RootC.root_cert_id from trusted_root_certificate RootC
																where RootC.subject_dn = TRC.issuer_dn and RootC.subject_key_identifier = TRC.subject_key_identifier limit 1) and
											RC.serial_number = TRC.cert_serial),
		cast(TRC.last_modify_date at time zone ''utc'' as timestamptz),
		TRC.crl_data,
		cast(TRC.when_created_crl at time zone ''utc'' as timestamptz),
		cast(TRC.when_next_crl at time zone ''utc'' as timestamptz),
		TRC.crl_update_period_sec,
		TRC.crl_access_point
	from trusted_root_certificate TRC
	where ' || root_cert_list_filter_ || ' and TRC.is_active = true
	order by TRC.subject_cn asc limit ' || quote_nullable(max_count_in) || ';';

end;
$$
language plpgsql;