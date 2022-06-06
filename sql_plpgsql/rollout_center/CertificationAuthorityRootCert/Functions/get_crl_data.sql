-- function get_crl_data(bigint)

drop function get_crl_data(bigint);

create or replace function get_crl_data(IN root_cert_id_in bigint, OUT crl_data_out varchar)
as
$$
declare
	subject_id_ bigint;
begin
	-- Получаем ID текущего сертификата пользователя
	select into subject_id_ get_current_user_cert_id();

	if not exists(select from trusted_root_certificate TRC where TRC.root_cert_id = root_cert_id_in) then
		execute 'select add_last_error(-8);'; -- Объекта с данным идентификатором нет.
		return;
	end if;

	if not check_permission_to_object_type(subject_id_, 'ca_certificate', null, 'r')
	then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	-- Получаем данные сертификата из таблицы trusted_root_certificate
	select TRC.crl_data
	from trusted_root_certificate TRC
	where TRC.root_cert_id = root_cert_id_in
	into crl_data_out;

end;
$$
language plpgsql;