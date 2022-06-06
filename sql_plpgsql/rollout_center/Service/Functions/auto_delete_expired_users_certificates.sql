-- Function: auto_delete_expired_users_certificates()

DROP FUNCTION auto_delete_expired_users_certificates();

CREATE OR REPLACE FUNCTION auto_delete_expired_users_certificates()
	RETURNS void AS
$BODY$
declare
	subject_id_ bigint;
	cert_ids_to_delete_ bigint[];
	cert_id_to_delete_ bigint;
	time_now_ timestamp with time zone;
	keep_user_cert_before_delete_days_ bigint;

	cert_serial_num_ varchar;
	cert_issuer_dn_ varchar;
	cert_ikeyid_ bytea;
begin
	-- Получаем ID текущего сертификата пользователя
	select into subject_id_ get_current_user_cert_id();

	-- Проверяем права доступа на запись сертификатов
	if not check_permission_to_object_type(subject_id_, 'certificate', null, 'w') then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	select into time_now_ now();
	select get_int_tlsgtw_option('system', 'keep_user_cert_after_expire_in_days') into keep_user_cert_before_delete_days_;

	-- удаляем только те сертификаты, срок действия которых истек позже чем keep_user_cert_before_delete_days_ дней назад
	select array(select UC.cert_id from users_certificates UC
								where (select extract(day from time_now_ - UC.valid_to)) > keep_user_cert_before_delete_days_)
	into cert_ids_to_delete_;

	foreach cert_id_to_delete_ in array cert_ids_to_delete_
	loop
		delete from users_certificates UC where UC.cert_id = cert_id_to_delete_
		returning UC.issuer_dn, UC.issuer_key_identifier, UC.serial_number into cert_issuer_dn_, cert_ikeyid_, cert_serial_num_;

		-- TODO: удаление разрешений и обновление версий должно быть в автоматической функции проверки срока действия и отозванности сертификата
		-- перенести после согласования логики этой функции
		-- nginxdb_proxy
		delete from nginxdb_proxy_permissions NDBP
		where NDBP.cert_serial_number = cert_serial_num_ and
					NDBP.iid = (select from nginxdb_proxy_issuer NDBI where NDBI.issuer_dn = cert_issuer_dn_);

		-- меняем версию БД
		update db_version set dbversion = dbversion + 1;
	end loop;

	return;
end;
$BODY$
	LANGUAGE plpgsql VOLATILE
	COST 100;
ALTER FUNCTION auto_delete_expired_users_certificates()
OWNER TO postgres;
