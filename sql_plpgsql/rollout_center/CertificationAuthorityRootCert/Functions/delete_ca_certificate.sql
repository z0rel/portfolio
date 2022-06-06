-- function delete_ca_certificate(bigint);

drop function delete_ca_certificate(bigint);

create or replace function delete_ca_certificate(IN ca_id_in bigint, OUT subject_name_out varchar) as
	--TODO добавить проверку, что удаляемый корневой сертификат не является тем, который подписал транспортный сертификат
$$
declare
	subject_id_ bigint;
	ikeyid_ bytea;
	deleted_serial_ varchar;
	iid_ int;
begin
	-- Получаем ID текущего сертификата пользователя
	select into subject_id_ get_current_user_cert_id();

	if not exists (select * from trusted_root_certificate TRC where TRC.root_cert_id = ca_id_in)
	then
		execute 'select add_last_error(3);'; -- Удаляемый объект не существует.
		return;
	end if;

	if not check_permission_to_object_type(subject_id_, 'ca_certificate', null, 'w')
	then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	-- удаляем корневой сертификат УЦ
	delete from trusted_root_certificate TRC where TRC.root_cert_id = ca_id_in
	returning TRC.subject_dn, TRC.subject_key_identifier into subject_name_out, ikeyid_;

	-- чистим разрешения для nginx для удаляемых пользователей
	for deleted_serial_ in (select UC.serial_number from users_certificates UC where UC.issuer_dn = subject_name_out and UC.issuer_key_identifier = ikeyid_)
	loop
		delete from nginxdb_proxy_permissions NDBP
		where NDBP.iid = (select NDBI.iid from nginxdb_proxy_issuer NDBI where NDBI.issuer_dn = subject_name_out) and
					NDBP.cert_serial_number = deleted_serial_;
	end loop;

	-- удаляем сертификаты пользователей, заверенные данным корневым сертификатов, и все их права (удаляются по внешнему ключу)
	delete from users_certificates UC where UC.issuer_dn = subject_name_out and UC.issuer_key_identifier = ikeyid_;

	-- ngingxdb_proxy
	if not exists (select * from trusted_root_certificate TRC where TRC.subject_dn = subject_name_out)
	then
		-- если последний корневой с таким subject_dn - удаляем из таблиц для nginxdb_proxy
		delete from nginxdb_proxy_issuer NDBI where NDBI.issuer_dn = subject_name_out returning NDBI.iid into iid_;
		delete from nginxdb_proxy_permissions NDBP where NDBP.iid = iid_;
	end if;

	-- меняем версию БД
	update db_version set dbversion = dbversion + 1;
end;
$$
language plpgsql;
