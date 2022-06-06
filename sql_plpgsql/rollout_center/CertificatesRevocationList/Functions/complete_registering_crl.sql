-- function complete_registering_crl(varchar, bytea)

drop function complete_registering_crl(varchar, bytea);

create or replace function complete_registering_crl(issuer_in varchar, ikeyid_in bytea) returns void as
$$
declare
	trusted_ca_id_ bigint;
	exist_crl_create_date_ timestamptz;
	new_crl_create_data_ timestamptz;
	new_crl_when_next_ timestamptz;

	resource_ bigint;
	cert_id_ bigint;
	serial_number_ varchar;

	iid_ int;
	rid_ int;

	root_cert_id_ bigint;
	root_serial_number_ varchar;
	root_subject_ varchar;
	root_ikeyid_ bytea;
begin
	if not exists (select relname from pg_class where relname = 'tt_crl_info') or
			not exists (select relname from pg_class where relname = 'tt_revoked_serials')
	then
		execute 'select add_last_error(-4);'; -- Временная таблица не найдена
		return;
	end if;

	select when_created, when_next from tt_crl_info where issuer_dn = issuer_in and ikeyid = ikeyid_in
	into new_crl_create_data_, new_crl_when_next_;
	if (new_crl_create_data_ is null or new_crl_when_next_ is null)
	then
		execute 'select add_last_error(-13);'; -- Объекта с данным строковым идентификатором нет
		return;
	end if;

	select TRC.root_cert_id, TRC.when_created_crl
	from trusted_root_certificate TRC
	where TRC.subject_dn = issuer_in and TRC.subject_key_identifier = ikeyid_in and TRC.is_active
	into trusted_ca_id_, exist_crl_create_date_;
	if trusted_ca_id_ is null
	then
		execute 'select add_last_error(-26);'; -- Корневой сертификат с данным именем не зарегестрирован
		return;
	end if;

	if not (exist_crl_create_date_ is null) and (new_crl_create_data_ < exist_crl_create_date_)
	then
		execute 'select add_last_error(-25);'; -- Для данного издателя уже зарегистрирован более свежий CRL
		return;
	end if;

	-- восстанавливаем разрешения сертификатов для nginxdb_proxy:
	-- если действие доверенного сертификата было приостановлено, но с новым crl продолжено,
	-- необходимо вернуть права сертификатам, выпущенным данным УЦ
	for root_cert_id_ in (select TRC.root_cert_id from trusted_root_certificate TRC
											where TRC.cert_serial = any (array(select RC.serial_number from revoked_certs RC where RC.trusted_root_cert_id = trusted_ca_id_))
							 				EXCEPT
											select TRC.root_cert_id from trusted_root_certificate TRC
											where TRC.subject_dn = issuer_in and TRC.subject_key_identifier = ikeyid_in and
											TRC.cert_serial = any (array(select TTRC.serial_number from tt_revoked_serials TTRC where TTRC.issuer_dn = issuer_in and TTRC.ikeyid = ikeyid_in)))
	loop
		select TRC.cert_serial, TRC.subject_dn, TRC.subject_key_identifier
		from trusted_root_certificate TRC
		where TRC.root_cert_id = root_cert_id_
		into root_serial_number_, root_subject_, root_ikeyid_;

		select into iid_ NDBI.iid
		from nginxdb_proxy_issuer NDBI
		where NDBI.issuer_dn = root_subject_;

		for cert_id_ in (select UC.cert_id from users_certificates UC where UC.issuer_dn = root_subject_ and UC.issuer_key_identifier = root_ikeyid_)
		loop
			select UC.serial_number from users_certificates UC where UC.cert_id = cert_id_ into serial_number_;

			for resource_ in (select URAE.resource_id from user_resources_access_entries URAE where URAE.user_cert_id = cert_id_ and URAE.access_allowed = true)
			loop
				select into rid_ NDBR.rid
				from nginxdb_proxy_resources NDBR
				where NDBR.external_addr = (select PR.external_address from proxy_resources PR where PR.resource_id = resource_);

				if not (rid_ is null) and not (iid_ is null) and not (serial_number_ is null)
				then
					insert into nginxdb_proxy_permissions (iid, rid, cert_serial_number) values (iid_,rid_,serial_number_);
				end if;
			end loop;
		end loop;
	end loop;

	-- восстанавливаем разрешения сертификатов для nginxdb_proxy:
	-- цикл по сертификатам, которые были отозваны ранее, но в новый crl не попали (действие приостановленного сертификата продолжено)
	for cert_id_ in (select UC.cert_id from users_certificates UC
										where UC.serial_number = any (array(select RC.serial_number from revoked_certs RC where RC.trusted_root_cert_id = trusted_ca_id_))
										EXCEPT
										select UC.cert_id from users_certificates UC
										where UC.issuer_dn = issuer_in and UC.issuer_key_identifier = ikeyid_in and
										UC.serial_number = any (array(select TTRC.serial_number from tt_revoked_serials TTRC where TTRC.issuer_dn = issuer_in and TTRC.ikeyid = ikeyid_in)))
	loop
		select UC.serial_number
		from users_certificates UC
		where UC.cert_id = cert_id_
		into serial_number_;

		select into iid_ NDBI.iid
		from nginxdb_proxy_issuer NDBI
		where NDBI.issuer_dn = issuer_in;

		for resource_ in (select URAE.resource_id from user_resources_access_entries URAE where URAE.user_cert_id = cert_id_ and URAE.access_allowed = true)
		loop
			select into rid_ NDBR.rid
			from nginxdb_proxy_resources NDBR
			where NDBR.external_addr = (select PR.external_address from proxy_resources PR where PR.resource_id = resource_);

			if not (rid_ is null) and not (iid_ is null) and not (serial_number_ is null)
			then
				insert into nginxdb_proxy_permissions (iid, rid, cert_serial_number) values (iid_,rid_,serial_number_);
			end if;
		end loop;

	end loop;

	-- удаляем права сертификатов для nginxdb_proxy тех сертификатов, чей корневой сертификат УЦ был отозван
	for root_cert_id_ in (select TRC.root_cert_id from trusted_root_certificate TRC
											where TRC.subject_dn = issuer_in and TRC.subject_key_identifier = ikeyid_in and
											TRC.cert_serial = any (array(select TTRC.serial_number from tt_revoked_serials TTRC where TTRC.issuer_dn = issuer_in and TTRC.ikeyid = ikeyid_in)))
									 		EXCEPT
											select TRC.root_cert_id from trusted_root_certificate TRC
											where TRC.cert_serial = any (array(select RC.serial_number from revoked_certs RC where RC.trusted_root_cert_id = trusted_ca_id_))
	loop
		-- событие: корневой сертификат отозван
		perform register_event('ca_cert_revoked', root_cert_id_, null, true, '{}');

		select TRC.cert_serial, TRC.subject_dn, TRC.subject_key_identifier
		from trusted_root_certificate TRC
		where TRC.root_cert_id = root_cert_id_
		into root_serial_number_, root_subject_, root_ikeyid_;

		select into iid_ NDBI.iid
		from nginxdb_proxy_issuer NDBI
		where NDBI.issuer_dn = root_subject_;

		for cert_id_ in (select UC.cert_id from users_certificates UC where UC.issuer_dn = root_subject_ and UC.issuer_key_identifier = root_ikeyid_)
		loop
			select UC.serial_number from users_certificates UC where UC.cert_id = cert_id_ into serial_number_;
			delete from nginxdb_proxy_permissions NDBP where NDBP.iid = iid_ and NDBP.cert_serial_number = serial_number_;
		end loop;
	end loop;

	-- удаляем разрешения сертификатов для nginxdb_proxy:
	-- цикл по сертификатам, которые попали в новый crl
	for cert_id_ in (select UC.cert_id from users_certificates UC
										where UC.issuer_dn = issuer_in and UC.issuer_key_identifier = ikeyid_in and
										UC.serial_number = any (array(select TTRC.serial_number from tt_revoked_serials TTRC where TTRC.issuer_dn = issuer_in and TTRC.ikeyid = ikeyid_in)))
									 	EXCEPT
										select UC.cert_id from users_certificates UC
										where UC.serial_number = any (array(select RC.serial_number from revoked_certs RC where RC.trusted_root_cert_id = trusted_ca_id_))
	loop
		select UC.serial_number
		from users_certificates UC
		where UC.cert_id = cert_id_
		into serial_number_;

		select into iid_ NDBI.iid
		from nginxdb_proxy_issuer NDBI
		where NDBI.issuer_dn = issuer_in;

		delete from nginxdb_proxy_permissions NDBP where NDBP.iid = iid_ and NDBP.cert_serial_number = serial_number_;
	end loop;

	-- обновляем данные crl
	update trusted_root_certificate set
		when_created_crl = coalesce(new_crl_create_data_, when_created_crl),
		when_next_crl = coalesce(new_crl_when_next_, when_next_crl)
	where subject_dn = issuer_in and subject_key_identifier = ikeyid_in;

	-- удаляем старые записи серийных номеров для данного УЦ
	-- они могут содержать серийные номера сертификатов, действие которых было приостановлено
	delete from revoked_certs RC where RC.trusted_root_cert_id = trusted_ca_id_;

	insert into revoked_certs (trusted_root_cert_id, serial_number)
	select
		trusted_ca_id_,
		serial_number
		from tt_revoked_serials where issuer_dn = issuer_in and ikeyid = ikeyid_in;

	-- событие: загружен новый CRL
	perform register_event('ca_crl_uploaded', (select TRC.root_cert_id from trusted_root_certificate TRC where TRC.subject_dn = issuer_in and TRC.subject_key_identifier = ikeyid_in), null, true, '{}');

	-- меняем версию БД
	update db_version set dbversion = dbversion + 1;

	-- удаляем временные таблицы
	drop table if exists tt_crl_info;
	drop table if exists tt_revoked_serials;
end;
$$
language plpgsql;