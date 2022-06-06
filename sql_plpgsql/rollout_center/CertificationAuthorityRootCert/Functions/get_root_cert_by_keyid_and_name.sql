-- Function: get_root_cert_by_keyid_and_name(varchar, bytea)

DROP FUNCTION get_root_cert_by_keyid_and_name(varchar, bytea);

CREATE OR REPLACE FUNCTION get_root_cert_by_keyid_and_name(
	IN issuer_name_in varchar,
	IN issuer_key_id_in bytea,
	OUT root_cert_out bytea)
 AS
$$
begin
	select TRC.root_cert
	from trusted_root_certificate TRC
	where TRC.subject_dn = issuer_name_in and TRC.subject_key_identifier = issuer_key_id_in and TRC.is_active
	into root_cert_out;

	if (root_cert_out is null)
	then
		execute 'select add_last_error(-26);'; -- Корневой сертификат с данным именем и идентификатором ключа не зарегестрирован
	end if;

end;
$$
language plpgsql
