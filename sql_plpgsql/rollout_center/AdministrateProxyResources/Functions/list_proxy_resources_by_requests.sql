-- Function: list_proxy_resources_by_requests(varchar, bigint[])

DROP FUNCTION list_proxy_resources_by_requests(varchar, bigint[]);

CREATE OR REPLACE FUNCTION list_proxy_resources_by_requests(
	search_value_in varchar,
	requests_users_ids bigint[])
	RETURNS TABLE(
		r_id bigint,
		r_name varchar,
		r_description varchar,
		r_internal_address varchar,
		r_external_address varchar,
		r_auth_mode tls_type,
		r_nginx_subfilter varchar,
		r_last_modify_date timestamptz) AS
$BODY$
declare
	subject_id_ bigint;
	resources_filter text;
	request_user_id_ bigint;
	user_requested_resources_ bigint[];
	requested_resources_filter_ bigint[];
begin
	-- Получаем ID текущего сертификата пользователя
	select into subject_id_ get_current_user_cert_id();

	if not check_permission_to_object_type(subject_id_, 'proxy_resource', null, 'r')
	then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	resources_filter:='';
	if not (search_value_in is null)
	then
		resources_filter := resources_filter ||
							'(PR.resource_name ~* concat_ws(''.*'', ' || quote_nullable(search_value_in) || ', ''.*'')';
	end if;

	if not (requests_users_ids is null)
	then
		-- все ресурсы в системе
		requested_resources_filter_ := array(select PR.resource_id from proxy_resources PR);

		-- формируем список ресурсов для выборки
		-- на основании списка запросов:
		-- должны быть выданы только те ресурсы, к которым запросили доступ все пользователи (пересечение множеств запрошенных ресурсов)
		foreach request_user_id_ in array requests_users_ids
		loop
			select array(select RRE.requested_resource_id from resources_requests_entries RRE where RRE.user_cert_id = request_user_id_) into user_requested_resources_;
			requested_resources_filter_ := array_intersect(requested_resources_filter_, user_requested_resources_);
		end loop;

		if (char_length(resources_filter) > 0)
		then
			resources_filter := resources_filter || ' and ';
		end if;

		resources_filter := resources_filter || ' PR.resource_id = any (ARRAY[' || array_to_string(requested_resources_filter_, ',', 'null') || ']::bigint[])';
	end if;


	if (char_length(resources_filter) = 0)
	then
		resources_filter := ' true';
	end if;

	return query execute 'select
				PR.resource_id,
				PR.resource_name,
				PR.resource_description,
				PR.internal_address,
				PR.external_address,
				PR.auth_mode,
				PR.nginxdb_proxy_subfilter,
				cast(PR.last_modify_date at time zone ''utc'' as timestamptz)
				from proxy_resources PR
				where ' || resources_filter || ' and PR.auth_mode = ''twoSideTLS''
				order by PR.resource_id;';

end;
$BODY$
	LANGUAGE plpgsql VOLATILE
	COST 100
	ROWS 1000;
ALTER FUNCTION list_proxy_resources_by_requests(varchar, bigint[])
	OWNER TO postgres;
COMMENT ON FUNCTION list_proxy_resources_by_requests(varchar, bigint[])
IS 'Возвращает список проксируемых ресурсов, к которым запросили доступ все пользователи из фильтра (пересечение множеств запрошенных ресурсов)';
