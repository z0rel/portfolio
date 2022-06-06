-- Function: list_proxy_resources_for_admin(varchar, tls_type)

DROP FUNCTION list_proxy_resources_for_admin(varchar, tls_type);

CREATE OR REPLACE FUNCTION list_proxy_resources_for_admin(
	search_value_in varchar,
	auth_mode_filter_in tls_type)
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
							'(PR.resource_name ~* concat_ws(''.*'', ' || quote_nullable(search_value_in) || ', ''.*'') or
							PR.resource_description ~* concat_ws(''.*'', ' || quote_nullable(search_value_in) || ', ''.*'') or
							PR.internal_address ~* concat_ws(''.*'', ' || quote_nullable(search_value_in) || ', ''.*'') or
							PR.external_address ~* concat_ws(''.*'', ' || quote_nullable(search_value_in) || ', ''.*''))';
	end if;

	if not (auth_mode_filter_in is null)
	then
		if (char_length(resources_filter) > 0)
		then
			resources_filter := resources_filter || ' and ';
		end if;

		resources_filter := resources_filter || ' PR.auth_mode = ' || quote_nullable(auth_mode_filter_in);
	end if;

	if (char_length(resources_filter) = 0)
	then
		resources_filter := ' true';
	end if;

	-- для администратора выводим список всех ресурсов с указанным типом аутентификации
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
				where ' || resources_filter || '
				order by PR.resource_id;';

end;
$BODY$
	LANGUAGE plpgsql VOLATILE
	COST 100
	ROWS 1000;
ALTER FUNCTION list_proxy_resources_for_admin(varchar, tls_type)
	OWNER TO postgres;
COMMENT ON FUNCTION list_proxy_resources_for_admin(varchar, tls_type)
IS 'Возвращает список проксируемых ресурсов для администратора.';
