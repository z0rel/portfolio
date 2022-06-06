-- function list_registered_crl(varchar, bytea)

drop function list_registered_crl(varchar, bytea);

create or replace function list_registered_crl(issuer_in varchar, ikeyid_in bytea)
	returns table (issuer_DN varchar, ikeyid bytea, when_created_crl timestamptz, when_next_crl timestamptz, root_cert varchar) as
$$
begin
	if (issuer_in is null and ikeyid_in is null)
	then
		return query select TRC.subject_dn, TRC.subject_key_identifier, TRC.when_created_crl, TRC.when_next_crl, TRC.root_cert
		from trusted_root_certificate TRC where TRC.is_active;
	elseif not (issuer_in is null) and not (ikeyid_in is null)
	then
		return query select TRC.subject_dn, TRC.subject_key_identifier, TRC.when_created_crl, TRC.when_next_crl, TRC.root_cert
		from trusted_root_certificate TRC where TRC.subject_dn = issuer_in and TRC.subject_key_identifier = ikeyid_in and TRC.is_active;
	else
		execute 'select add_last_error(-3);'; -- Передаваемые аргументы взаимоисключающие.
	end if;
end;
$$
language plpgsql;