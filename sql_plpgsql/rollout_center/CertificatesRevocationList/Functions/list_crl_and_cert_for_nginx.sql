-- function list_crl_and_cert_for_nginx()

drop function list_crl_and_cert_for_nginx();

create or replace function list_crl_and_cert_for_nginx()
	returns table (issuer_DN varchar, ikeyid bytea, root_cert varchar, crl varchar) as
$$
begin
	-- именем издателя CRL будет subject_name корневого сертификата
	return query select TRC.subject_dn, TRC.subject_key_identifier, TRC.root_cert, TRC.crl_data from trusted_root_certificate TRC
	where TRC.is_active and
				not exists (select * from revoked_certs RC where RC.serial_number = TRC.cert_serial and RC.trusted_root_cert_id = TRC.root_cert_id);
end;
$$
language plpgsql;