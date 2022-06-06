-- Function: auto_check_expired_certificates_and_crl()

DROP FUNCTION auto_check_expired_certificates_and_crl();

CREATE OR REPLACE FUNCTION auto_check_expired_certificates_and_crl()
	RETURNS void AS
$$
declare
	subject_id_ bigint;

	time_now_ timestamp with time zone;
	ca_cert_and_crl_expire_in_days_ bigint;

	root_cert_id_ bigint;
	root_cert_valid_to_ timestamptz;
	root_cert_subject_ varchar;
	root_cert_subject_cn varchar;
	root_cert_skey_id_ bytea;
	root_cert_crl_when_next_ timestamptz;
begin
	-- Получаем ID текущего сертификата пользователя
	select into subject_id_ get_current_user_cert_id();

	-- Проверяем права доступа на запись сертификатов
	if not check_permission_to_object_type(subject_id_, 'ca_certificate', null, 'r') then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	select into time_now_ now();
	select get_int_tlsgtw_option('system', 'ca_cert_and_crl_expire_in_days') into ca_cert_and_crl_expire_in_days_;

	-- проверяем активные сертификаты, срок действия которых истекает  меньше чем через ca_cert_and_crl_expire_in_days_ дней
	for root_cert_id_ in (select TRC.root_cert_id from trusted_root_certificate TRC
												where (select extract(day from TRC.valid_to - time_now_)) < ca_cert_and_crl_expire_in_days_
															and TRC.is_active = true)
	loop
		select TRC.valid_to, TRC.subject_dn, TRC.subject_key_identifier
		from trusted_root_certificate TRC
		where TRC.root_cert_id = root_cert_id_
		into root_cert_valid_to_, root_cert_subject_, root_cert_skey_id_;

		-- если срок действия истек
		if ( extract(second from root_cert_valid_to_ - time_now_) < 0 )
		then
			-- событие: корневой сертификат истек
			perform register_event('ca_cert_expired', root_cert_id_, null, true, '{}');

			-- nginxdb_proxy
			delete from nginxdb_proxy_permissions NDBP
			where NDBP.iid = (select NDBI.iid from nginxdb_proxy_issuer NDBI where NDBI.issuer_dn = root_cert_subject_) and
						NDBP.cert_serial_number = any (select UC.serial_number from users_certificates UC where UC.issuer_dn = root_cert_subject_ and UC.issuer_key_identifier = root_cert_skey_id_);
			-- меняем версию БД
			update db_version set dbversion = dbversion + 1;
		else
			-- до конца действия меньше чем ca_cert_and_crl_expire_in_days_ дней
			-- событие: корневой сертификат истекает
			perform register_event('ca_cert_expiring', root_cert_id_, null, true, '{}');
		end if;

	end loop;

	-- проверяем сертификаты, рекомендуемый срок действия CRL которых истекает меньше чем через ca_cert_and_crl_expire_in_days_ дней
	for root_cert_id_ in (select TRC.root_cert_id from trusted_root_certificate TRC
												where (select extract(day from TRC.when_next_crl - time_now_)) < ca_cert_and_crl_expire_in_days_
															and TRC.is_active = true)
	loop
		select TRC.when_next_crl, TRC.subject_cn
		from trusted_root_certificate TRC
		where TRC.root_cert_id = root_cert_id_
		into root_cert_crl_when_next_, root_cert_subject_cn;

		-- если срок действия истек
		if ( extract(second from root_cert_crl_when_next_ - time_now_) < 0 )
		then
			-- событие: CRL корневого сертификата истек
			perform register_event('ca_crl_expired', root_cert_id_, null, true, array[root_cert_subject_cn]::varchar[]);
		else -- до конца действия меньше чем ca_cert_and_crl_expire_in_days_ дней

			-- событие: CRL корневого сертификата истекает
			perform register_event('ca_crl_expiring', root_cert_id_, null, true, array[root_cert_subject_cn]::varchar[]);
		end if;

	end loop;

	return;
end;
$$
	language plpgsql volatile

