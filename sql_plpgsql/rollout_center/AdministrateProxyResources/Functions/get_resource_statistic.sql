-- Function: get_resource_statistic(bigint)

DROP FUNCTION  get_resource_statistic(bigint);

create or replace function get_resource_statistic(
	IN resource_id_in bigint,

	OUT requests_count_out bigint,
	OUT users_count_out bigint) as
$$
declare
	subject_id_ bigint;
begin
	-- Получаем ID текущего сертификата пользователя
	select into subject_id_ get_current_user_cert_id();

	if not check_permission_to_object(subject_id_, resource_id_in, null, 'w')
	then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	select count(*) from user_resources_access_entries URAE
	where URAE.access_allowed = true and URAE.resource_id = resource_id_in into users_count_out;

	select count(*) from resources_requests_entries RRE
	where RRE.requested_resource_id = resource_id_in into requests_count_out;
end;
$$
language plpgsql

