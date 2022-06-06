-- Function: list_access_proxy_resources_for_user(varchar, bigint)

DROP FUNCTION list_access_proxy_resources_for_user(varchar, bigint);

CREATE OR REPLACE FUNCTION list_access_proxy_resources_for_user(
	search_value_in varchar,
	user_cert_id_in bigint)
	RETURNS TABLE(
		r_id bigint,
		r_name varchar,
		r_description varchar,
		r_internal_address varchar,
		r_external_address varchar,
		r_auth_mode tls_type,
		r_nginx_subfilter varchar,
		r_last_modify_date timestamptz,
		r_access_allowed boolean) AS
$BODY$
declare
	subject_id_ bigint;
	resources_filter text;
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
							'(PR.resource_name ~* concat_ws(''.*'', ' || quote_nullable(search_value_in) || ', ''.*''))';
	end if;

	if (char_length(resources_filter) = 0)
	then
		resources_filter := ' true';
	end if;

	if (user_cert_id_in is null)
	then
		-- для администратора выводим список всех ресурсов с двусторонней аутентификацией
		return query execute 'select
				PR.resource_id,
				PR.resource_name,
				PR.resource_description,
				PR.internal_address,
				PR.external_address,
				PR.auth_mode,
				PR.nginxdb_proxy_subfilter,
				cast(PR.last_modify_date at time zone ''utc'' as timestamptz),
				false
				from proxy_resources PR
				where ' || resources_filter || ' and PR.auth_mode = ''twoSideTLS''
				order by PR.resource_id;';
	else
		-- для администратора выводим список всех ресурсов с правами доступа к ресурсам
		-- с двусторонней аутентификацией (разрешшен/запрещен/можно запросить) для пользователя с указанным идентификатором
		return query execute 'select
				PR.resource_id,
				PR.resource_name,
				PR.resource_description,
				PR.internal_address,
				PR.external_address,
				PR.auth_mode,
				PR.nginxdb_proxy_subfilter,
				cast(PR.last_modify_date at time zone ''utc'' as timestamptz),
				URAE.access_allowed
				from proxy_resources PR
				left join user_resources_access_entries URAE on URAE.resource_id = PR.resource_id and URAE.user_cert_id = ' || user_cert_id_in || '
				where ' || resources_filter || ' and PR.auth_mode = ''twoSideTLS''
				order by PR.resource_id;';
	end if;

end;
$BODY$
	LANGUAGE plpgsql VOLATILE
	COST 100
	ROWS 1000;
ALTER FUNCTION list_access_proxy_resources_for_user(varchar, bigint)
	OWNER TO postgres;
COMMENT ON FUNCTION list_access_proxy_resources_for_user(varchar, bigint)
IS 'Возвращает список проксируемых ресурсов с правами доступа для пользователя';
