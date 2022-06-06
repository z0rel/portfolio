-- function add_to_crl_registration(varchar, bytea, varchar[])

drop function add_to_crl_registration(varchar, bytea, varchar[]);

create or replace function add_to_crl_registration(issuer_in varchar, ikeyid_in bytea, revoked_sn varchar[]) returns void as
$$
declare
	revoked_serial_ varchar;
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

	if not (revoked_sn is null)
	then
		foreach revoked_serial_ in array revoked_sn
		loop
			insert into tt_revoked_serials (issuer_dn, ikeyid, serial_number) values (issuer_in, ikeyid_in, revoked_serial_);
		end loop;
	end if;

end;
$$
language plpgsql;