-- function save_crl(varchar, bytea, varchar)

drop function save_crl(varchar, bytea, varchar);

create or replace function save_crl(issuer_in varchar, ikeyid_in bytea, crl_content_in varchar) returns void as
$$
begin
	if not exists (select TRC.root_cert_id from trusted_root_certificate TRC where TRC.subject_dn = issuer_in and TRC.subject_key_identifier = ikeyid_in and TRC.is_active)
	then
		execute 'select add_last_error(-26);'; -- Корневой сертификат с данным именем и идентификатором ключа не зарегестрирован
		return;
	end if;

	if (crl_content_in is null)
	then
		execute 'select add_last_error(-6);'; -- NULL недопустим
		return;
	end if;

	update trusted_root_certificate set crl_data = crl_content_in where subject_dn = issuer_in and subject_key_identifier = ikeyid_in;
end;
$$
language plpgsql;