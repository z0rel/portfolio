-- Function: get_tlsgtw_info()

drop function get_tlsgtw_info();

create or replace function get_tlsgtw_info(
	OUT current_user_roles varchar[],
	OUT requests_count_out bigint,
	OUT last_request_date_out timestamptz,
	OUT rules_count bigint, -- TODO добавить заполнение параметра после реализации хранилища правил
	OUT resources_count bigint) as
$$
declare
	subject_id_ bigint;
begin
	-- Получаем ID текущего сертификата пользователя
	select into subject_id_ get_current_user_cert_id();

	if not check_permission_to_object_type(subject_id_, 'proxy_resource', null, 'r')
	then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	if not check_permission_to_object_type(subject_id_, 'certificate', null, 'r')
	then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	-- роли текущего пользователя
	select array(select R.string_id
		from user_roles_entries URE
		inner join roles R on URE.role_id = R.role_id
		where URE.user_cert_id = subject_id_) into current_user_roles;

	-- количество запросов к ресурсам (запросы, для которых есть записи в resources_requests_entries)
	select count(*) from resources_access_requests RAR
	where not (select count(*) from resources_requests_entries RRE where RRE.user_cert_id = RAR.user_cert_id) = 0
	into requests_count_out;

	-- дата последнего запроса
	select cast(RAR.request_date at time zone 'utc' as timestamptz) from resources_access_requests RAR
	where not (select count(*) from resources_requests_entries RRE where RRE.user_cert_id = RAR.user_cert_id) = 0
	order by RAR.request_date desc limit 1 into last_request_date_out;

	select count(*) from proxy_resources PR into resources_count;
end;
$$
language plpgsql;
