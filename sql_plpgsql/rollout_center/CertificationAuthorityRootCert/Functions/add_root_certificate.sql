-- function add_root_certificate(varchar, varchar, bytea, varchar, varchar, character varying(64), character varying(64), character varying(1024), timestamptz, timestamptz);

drop function add_root_certificate(varchar, varchar, bytea, varchar, varchar, character varying(64), character varying(64), character varying(1024), timestamptz, timestamptz);

create or replace function add_root_certificate(
	IN ca_issuer_in varchar,
	IN ca_subject_in varchar,
	IN subject_key_identifier_in bytea,
	IN ca_cert_serial_in varchar,
	IN root_certificate_in varchar,

	IN ca_cert_issuer_cn_in character varying(64),
	IN ca_cert_subject_cn_in character varying(64),
	IN ca_cert_subject_o_in character varying(1024),
	IN ca_cert_valid_from_in timestamptz,
	IN ca_cert_valid_to_in timestamptz,

	OUT ca_cert_id_out bigint,
	OUT last_modify_out timestamptz)
	as
$$
declare
	subject_id_ bigint;
	is_cert_active_ boolean;
begin
	if ca_issuer_in is null or
		ca_subject_in is null or
		subject_key_identifier_in is null or
		ca_cert_serial_in is null or
		root_certificate_in is null or
		ca_cert_issuer_cn_in is null or
		ca_cert_subject_cn_in is null or
		ca_cert_valid_from_in is null or
		ca_cert_valid_to_in is null
	then
		execute 'select add_last_error(-6);'; -- NULL недопустим.
		return;
	end if;

	-- Получаем ID текущего сертификата пользователя
	select into subject_id_ get_current_user_cert_id();

	if not check_permission_to_object_type(subject_id_, 'ca_certificate', null, 'w')
	then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	if (now() >  ca_cert_valid_to_in)
	then
		execute 'select add_last_error(-7);'; -- Просроченный сертификат
		return;
	end if;

	select TRC.root_cert_id, cast(TRC.last_modify_date at time zone 'utc' as timestamptz), TRC.is_active
	from trusted_root_certificate TRC
	where TRC.subject_dn = ca_subject_in and TRC.subject_key_identifier = subject_key_identifier_in
	into ca_cert_id_out, last_modify_out, is_cert_active_;

	if not (ca_cert_id_out is NULL)
	then
		if (is_cert_active_)
		then
			execute 'select add_last_error(-29);'; -- Корневой сертификат уже зарегестрирован
		end if;

		return;
	end if;

	insert into trusted_root_certificate
	(
		subject_dn,
		subject_key_identifier,
		cert_serial,
		root_cert,
		issuer_dn,
		issuer_cn,
		subject_cn,
		subject_o,
		valid_from,
		valid_to
	)
	values
	(
		ca_subject_in,
		subject_key_identifier_in,
		ca_cert_serial_in,
		root_certificate_in,
		ca_issuer_in,
		ca_cert_issuer_cn_in,
		ca_cert_subject_cn_in,
		ca_cert_subject_o_in,
		ca_cert_valid_from_in,
		ca_cert_valid_to_in
	)
	returning root_cert_id, cast(last_modify_date at time zone 'utc' as timestamptz) into ca_cert_id_out, last_modify_out;

end;
$$
language plpgsql;