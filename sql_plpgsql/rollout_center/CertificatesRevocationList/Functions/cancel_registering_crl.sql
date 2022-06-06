-- function cancel_registering_crl(varchar, bytea)

drop function cancel_registering_crl(varchar, bytea);

create or replace function cancel_registering_crl(issuer_in varchar, ikeyid_in bytea) returns void as
$$
begin
	if not exists (select relname from pg_class where relname = 'tt_crl_info') or
			not exists (select relname from pg_class where relname = 'tt_revoked_serials')
	then
		execute 'select add_last_error(-4);'; -- Временная таблица не найдена
		return;
	end if;

	if not exists (select * from tt_crl_info where issuer_dn = issuer_in and ikeyid = ikeyid_in)
	then
		execute 'select add_last_error(-13);'; -- Объекта с данным строковым идентификатором нет
		return;
	end if;

	if not exists (select TRC.root_cert_id
									from trusted_root_certificate TRC
									where TRC.subject_dn = issuer_in and TRC.subject_key_identifier = ikeyid_in and TRC.is_active)
	then
		execute 'select add_last_error(-26);'; -- Корневой сертификат с данным именем и идентификатором ключа не зарегестрирован
		return;
	end if;

	drop table tt_crl_info;
	drop table tt_revoked_serials;
end;
$$
language plpgsql;