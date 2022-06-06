-- function activate_root_certificate(bigint, timestamptz);

drop function activate_root_certificate(bigint, timestamptz);

create or replace function activate_root_certificate(
IN root_cert_id_in bigint,
IN last_modify_in timestamptz,
OUT root_cert_subject_cn_out varchar)
as
$$
declare
	subject_id_ bigint;

	subject_dn_ varchar;
	last_modify_date_ timestamptz;
	valid_to_ timestamptz;
	is_root_cert_active_ boolean;
begin

	-- Получаем ID текущего сертификата пользователя
	select into subject_id_ get_current_user_cert_id();

	if not check_permission_to_object_type(subject_id_, 'ca_certificate', null, 'w')
	then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	select TRC.valid_to , TRC.last_modify_date, TRC.is_active, TRC.subject_dn
	from trusted_root_certificate TRC
	where TRC.root_cert_id = root_cert_id_in
	into valid_to_, last_modify_date_, is_root_cert_active_, subject_dn_;

	if (last_modify_date_ is null)
	then
		execute 'select add_last_error(-8);'; -- Объекта с данным идентификатором нет.
		return;
	end if;

	if (now() >  valid_to_)
	then
		execute 'select add_last_error(-7);'; -- Просроченный сертификат
		return;
	end if;

	if (last_modify_in != last_modify_date_)
	then
		execute 'select add_last_error(-20);'; -- Редактирование объекта невозможно, состояние было изменено.
		return;
	end if;

	if (is_root_cert_active_)
	then
		execute 'select add_last_error(-29);'; -- Корневой сертификат уже зарегестрирован
		return;
	end if;

	update trusted_root_certificate TRC
	set is_active = true,
			last_modify_date = now()
	where TRC.root_cert_id = root_cert_id_in
	returning TRC.subject_cn into root_cert_subject_cn_out;

	-- nginxdb_proxy
	if not exists (select * from nginxdb_proxy_issuer NDBI where NDBI.issuer_dn = subject_dn_)
	then
		-- именем издателя для пользовательских сертификатов является subject_dn корневого
		insert into nginxdb_proxy_issuer (issuer_dn) values (subject_dn_);
	end if;

end;
$$
language plpgsql;