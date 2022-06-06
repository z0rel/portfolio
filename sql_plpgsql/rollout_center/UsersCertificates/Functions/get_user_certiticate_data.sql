-- function get_user_certificate_data(bigint)

drop function get_user_certificate_data(bigint);

create or replace function get_user_certificate_data(cert_id_in bigint)
returns table (
	certificate_r bytea,
  certificate_ext_r varchar) as
$$
declare
	subject_id_ bigint;
begin
	-- Получаем ID текущего сертификата пользователя
	select into subject_id_ get_current_user_cert_id();

	if not exists(select from users_certificates UC where UC.cert_id = cert_id_in) then
		execute 'select add_last_error(-8);'; -- Объекта с данным идентификатором нет.
		return;
	end if;

	if not check_permission_to_object(subject_id_, cert_id_in, null, 'r') then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	-- Получаем данные сертификата и расширение файла из таблицы user_certificates
	return query (select
		UC.certificate,
		UC.certificate_ext
	from users_certificates UC
	where UC.cert_id = cert_id_in);

end;
$$
language plpgsql;