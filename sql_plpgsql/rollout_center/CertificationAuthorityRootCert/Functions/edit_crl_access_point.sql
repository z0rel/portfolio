-- funnction edit_crl_access_point(bigint, varchar, bigint, timestamptz);

drop function edit_crl_access_point(bigint, varchar, bigint, timestamptz);

create or replace function edit_crl_access_point(
	IN root_cert_id_in bigint,
	IN crl_access_point_url_in varchar,
	IN crl_update_period_sec_in bigint,
	IN last_modify_in timestamptz,
	OUT root_cert_subject_cn_out varchar,
	OUT last_modify_date_out timestamptz) as
$$
declare
	subject_id_ bigint;
	last_modify_date_ timestamptz;
begin
	-- Получаем ID текущего сертификата пользователя
	select into subject_id_ get_current_user_cert_id();

	if not check_permission_to_object_type(subject_id_, 'ca_certificate', null, 'w')
	then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	select TRC.last_modify_date from trusted_root_certificate TRC where TRC.root_cert_id = root_cert_id_in into last_modify_date_;
	if (last_modify_date_ is null)
	then
		execute 'select add_last_error(-8);'; -- Объекта с данным идентификатором нет.
		return;
	end if;

	if (last_modify_in != last_modify_date_)
	then
		execute 'select add_last_error(-20);'; -- Редактирование объекта невозможно, состояние было изменено.
		return;
	end if;

	-- меняем на новые
	update trusted_root_certificate TRC
	set crl_access_point = crl_access_point_url_in,
			crl_update_period_sec = crl_update_period_sec_in,
			last_modify_date = now()
	where TRC.root_cert_id = root_cert_id_in
	returning TRC.subject_cn, cast(TRC.last_modify_date at time zone 'utc' as timestamptz) into root_cert_subject_cn_out, last_modify_date_out;
end;
$$
language plpgsql;
