-- Function: add_user_certificate(bytea, character varying, character varying, character varying, bytea, character varying(64), character varying(64), character varying(1024), character(11), character varying(40), character varying(64), character varying(64), character varying(64), character varying(30), character varying(128), character varying(128), character(2), character varying(255), character varying(12), character(13), timestamp with time zone, timestamp with time zone, character varying, role_group);

DROP FUNCTION add_user_certificate(bytea, character varying, character varying, character varying, bytea, character varying(64), character varying(64), character varying(1024), character(11), character varying(40), character varying(64), character varying(64), character varying(64), character varying(30), character varying(128), character varying(128), character(2), character varying(255), character varying(12), character(13), timestamp with time zone, timestamp with time zone, character varying, role_group);

CREATE OR REPLACE FUNCTION add_user_certificate(
	IN certificate_in bytea,
	IN certificate_ext_in character varying,
	IN serial_number_in character varying,
	IN issuer_dn_in character varying,
	IN issuer_key_identifier_in bytea,
	IN issuer_cn_in character varying(64),
	IN subject_cn_in character varying(64),
	IN subject_o_in character varying(1024),
	IN subject_snils_in character(11),
	IN subject_surname_in character varying(40),
	IN subject_givenname_in character varying(64),
	IN subject_t_in character varying(64),
	IN subject_ou_in character varying(64),
	IN subject_street_in character varying(30),
	IN subject_l_in character varying(128),
	IN subject_s_in character varying(128),
	IN subject_c_in character(2),
	IN subject_email_in character varying(255),
	IN subject_inn_in character varying(12),
	IN subject_ogrn_in character(13),
	IN valid_from_in timestamp with time zone,
	IN valid_to_in timestamp with time zone,
	IN commentary_in character varying,
	IN cert_role_group role_group,

	OUT user_cert_id_out bigint,
	OUT last_permissions_modify_out timestamptz)
	AS
$BODY$
declare
	subject_id_ bigint;
	cert_type_ varchar;
	root_cert_id_ bigint;
	crl_data_ varchar;
begin
	if serial_number_in is null or
			issuer_dn_in is null or
			issuer_cn_in is null or
			issuer_key_identifier_in is null or
			certificate_in is null or
			certificate_ext_in is null or
			subject_cn_in is null or
			valid_from_in is null or
			valid_to_in is null
	then
		execute 'select add_last_error(-6);'; -- NULL недопустим.
		return;
	end if;

	-- Получаем ID текущего пользователя
	select into subject_id_ get_current_user_cert_id();

	if not check_permission_to_object_type(subject_id_, get_cert_type_by_role_group(cert_role_group), null, 'w')
	then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	select TRC.root_cert_id, TRC.crl_data
	from trusted_root_certificate TRC
	where TRC.subject_key_identifier = issuer_key_identifier_in and TRC.subject_dn = issuer_dn_in and TRC.is_active
	into root_cert_id_, crl_data_;
	if (root_cert_id_ is null)
	then
		execute 'select add_last_error(-26);'; -- Не найден корневой сертификат
		return;
	end if;

	if (crl_data_ is null)
	then
		execute 'select add_last_error(-31);'; -- Не найден CRL для корневого сертификата
		return;
	end if;

	if (now() >  valid_to_in)
	then
		execute 'select add_last_error(-7);';     -- Просроченный сертификат
		return;
	end if;

	if ( exists ( select * from revoked_certs RC
								inner join trusted_root_certificate TRC on RC.trusted_root_cert_id = TRC.root_cert_id
								where TRC.subject_dn = issuer_dn_in and TRC.subject_key_identifier = issuer_key_identifier_in and
											(RC.serial_number = serial_number_in or
											RC.serial_number = (select TRC.cert_serial
																					from trusted_root_certificate TRC
																					where TRC.subject_dn = issuer_dn_in and
																					TRC.subject_key_identifier = issuer_key_identifier_in))))
	then
		execute 'select add_last_error(-30);';     -- Отозванный сертификат
		return;
	end if;

	select UC.cert_id, cast(last_permissions_modify_date at time zone 'utc' as timestamptz)
	from  users_certificates UC
	where UC.issuer_dn = issuer_dn_in and UC.serial_number = serial_number_in
	into user_cert_id_out, last_permissions_modify_out;

	if not (user_cert_id_out is NULL)
	then
		if exists (select * from user_roles_entries URE inner join roles R on URE.role_id = R.role_id
								where R.group_type = cert_role_group and URE.user_cert_id = user_cert_id_out)
		then
			execute 'select add_last_error(5);'; -- Сертификат с данным именем издателя, серийным номером и типом уже зарегистрирован
		end if;

		return;
	end if;

	-- добавляем сертификат
	insert into users_certificates(certificate, certificate_ext, serial_number, issuer_dn, issuer_cn, issuer_key_identifier,
																 subject_cn, subject_o, subject_snils, subject_surname,
																 subject_givenname, subject_t, subject_ou, subject_street,
																 subject_l,  subject_s, subject_c, subject_email, subject_inn, subject_ogrn,
																 valid_from, valid_to, commentary)
	values (
	certificate_in,
	certificate_ext_in,
	serial_number_in,
	issuer_dn_in,
	issuer_cn_in,
	issuer_key_identifier_in,
	subject_cn_in,
	subject_o_in,
	subject_snils_in,
	subject_surname_in,
	subject_givenname_in,
	subject_t_in,
	subject_ou_in,
	subject_street_in,
	subject_l_in,
	subject_s_in,
	subject_c_in,
	subject_email_in,
	subject_inn_in,
	subject_ogrn_in,
	valid_from_in,
	valid_to_in,
	commentary_in)
	returning cert_id, cast(last_permissions_modify_date at time zone 'utc' as timestamptz) into user_cert_id_out, last_permissions_modify_out;

end;
$BODY$
	LANGUAGE plpgsql VOLATILE
	COST 100;
ALTER FUNCTION add_user_certificate(bytea, character varying, character varying, character varying, bytea, character varying(64), character varying(64), character varying(1024), character(11), character varying(40), character varying(64), character varying(64), character varying(64), character varying(30), character varying(128), character varying(128), character(2), character varying(255), character varying(12), character(13), timestamp with time zone, timestamp with time zone, character varying, role_group)
OWNER TO postgres;
