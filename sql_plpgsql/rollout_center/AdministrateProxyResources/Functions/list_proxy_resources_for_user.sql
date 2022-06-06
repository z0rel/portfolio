-- Function: list_proxy_resources_for_user(varchar, tls_type, boolean)

DROP FUNCTION list_proxy_resources_for_user(varchar, tls_type, boolean);

CREATE OR REPLACE FUNCTION list_proxy_resources_for_user(search_value_in varchar, tls_auth_mode tls_type, available_for_request boolean)
	RETURNS TABLE(
		r_id bigint,
		r_name varchar,
		r_description varchar,
		r_external_address varchar,
		r_auth_mode tls_type,
		r_is_pending boolean
) AS
$BODY$
declare
	subject_id_ bigint;
	resources_filter_ text;
	search_filter_ text;
	query_resources_ text;
	requested_resources_ bigint[];
begin
	-- Получаем ID текущего сертификата пользователя
	select into subject_id_ get_current_user_cert_id();

	if not check_permission_to_object_type(subject_id_, 'proxy_resource', null, 'r')
	then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	select array(select RRE.requested_resource_id from resources_requests_entries RRE where RRE.user_cert_id = subject_id_)_ into requested_resources_;

	resources_filter_:='';

	if (tls_auth_mode = 'oneSideTLS')
	then
		resources_filter_ := resources_filter_ || 'PR.auth_mode = ''oneSideTLS''';
	elseif (tls_auth_mode = 'twoSideTLS') and not (available_for_request is null) then
		-- ресурсы с двусторонним TLS
		-- ресурсы, доступ к которым запрошен
		resources_filter_ := resources_filter_ || '( ( PR.auth_mode = ''twoSideTLS'' and
		((PR.resource_id = any (ARRAY[' || array_to_string(requested_resources_, ',', 'null') || ']::bigint[])) or ';

		if (available_for_request) then
			-- ресурсы, к которым можно запросить доступ
			resources_filter_ := resources_filter_ ||
				'(URAE.user_cert_id is null and URAE.resource_id is null)) ) )';
		else
			-- ресурсы, к которым уже есть доступ
			resources_filter_ := resources_filter_ ||
				'(URAE.user_cert_id = '|| subject_id_ || ' and URAE.access_allowed = true)) ) or PR.auth_mode = ''oneSideTLS'' )';
		end if;
	else
		execute 'select add_last_error(-6);'; -- NULL недопустим
		return;
	end if;

	search_filter_:='';

	if not (search_value_in is null)
	then
		search_filter_ := ' and ' ||
							'(PR.resource_name ~* concat_ws(''.*'', ' || quote_nullable(search_value_in) || ', ''.*'') or
							PR.resource_description ~* concat_ws(''.*'', ' || quote_nullable(search_value_in) || ', ''.*'') or
							PR.internal_address ~* concat_ws(''.*'', ' || quote_nullable(search_value_in) || ', ''.*'') or
							PR.external_address ~* concat_ws(''.*'', ' || quote_nullable(search_value_in) || ', ''.*''))';
	end if;

	query_resources_:='select
				PR.resource_id,
				PR.resource_name,
				PR.resource_description,
				PR.external_address,
				PR.auth_mode,
				(select PR.resource_id  = any (ARRAY[' || array_to_string(requested_resources_, ',', 'null') || ']::bigint[]))
				from proxy_resources PR
				left join user_resources_access_entries URAE on URAE.resource_id = PR.resource_id and URAE.user_cert_id = ' || subject_id_ || '
				where (' || resources_filter_ || ')' || search_filter_ || '
				order by PR.resource_id;';

	return query execute query_resources_;

end;
$BODY$
	LANGUAGE plpgsql VOLATILE
	COST 100
	ROWS 1000;
ALTER FUNCTION list_proxy_resources_for_user(varchar, tls_type, boolean)
	OWNER TO postgres;
COMMENT ON FUNCTION list_proxy_resources_for_user(varchar, tls_type, boolean) IS 'Возвращает список запрошенных, разрешенных ресурсов или ресурсов к которым можно запросить доступ';
