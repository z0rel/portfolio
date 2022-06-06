-- function start_registering_crl(varchar, bytea, timestamptz, timestamptz)

drop function start_registering_crl(varchar, bytea, timestamptz, timestamptz);

create or replace function start_registering_crl(issuer_in varchar, ikeyid_in bytea, when_created_in timestamptz, when_next_in timestamptz)
	returns void as
$$
declare
	exist_crl_create_date timestamptz;
begin

	if not exists (select * from trusted_root_certificate TRC where TRC.subject_dn = issuer_in and TRC.subject_key_identifier = ikeyid_in and TRC.is_active)
	then
		execute 'select add_last_error(-26);'; -- Корневой сертификат с данным именем и идентификатором ключа не зарегестрирован
		return;
	end if;

	select TRC.when_created_crl from trusted_root_certificate TRC
	where TRC.subject_dn = issuer_in and TRC.subject_key_identifier = ikeyid_in
	into exist_crl_create_date;
	if not (exist_crl_create_date is null) and (when_created_in < exist_crl_create_date)
	then
		execute 'select add_last_error(-25);'; -- Для данного издателя уже зарегистрирован более свежий CRL
		return;
	end if;

	drop table if exists tt_crl_info;
	drop table if exists tt_revoked_serials;

	-- временная таблица с инофрмацией о crl
	create temp table tt_crl_info(issuer_dn varchar, ikeyid bytea, when_created timestamptz, when_next timestamptz, constraint tt_crl_pk primary key (issuer_dn, ikeyid));
	insert into tt_crl_info (issuer_dn, ikeyid, when_created, when_next) values (issuer_in, ikeyid_in, when_created_in, when_next_in);

	-- временная таблица с информацией об отозванныъ серийный номерах сертификатов
	create temp table tt_revoked_serials(issuer_dn varchar, ikeyid bytea, serial_number varchar, constraint tt_revoked_pk primary key (issuer_dn, ikeyid, serial_number));

end;
$$
language plpgsql;