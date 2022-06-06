-- Function: list_proxy_resources_with_requests()

DROP FUNCTION list_proxy_resources_with_requests();

CREATE OR REPLACE FUNCTION list_proxy_resources_with_requests()
	RETURNS TABLE(
		r_id bigint,
		r_name varchar,
		r_description varchar,
		r_requests_count bigint) AS
$BODY$
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

	return query select
				PR.resource_id,
				PR.resource_name,
				PR.resource_description,
				(select count(*) from resources_requests_entries RRE where RRE.requested_resource_id = PR.resource_id)
				from proxy_resources PR
				order by PR.resource_id;

end;
$BODY$
	LANGUAGE plpgsql VOLATILE
	COST 100
	ROWS 1000;
ALTER FUNCTION list_proxy_resources_with_requests()
	OWNER TO postgres;
COMMENT ON FUNCTION list_proxy_resources_with_requests()
IS 'Возвращает список проксируемых ресурсов и количество запросов к ним';
