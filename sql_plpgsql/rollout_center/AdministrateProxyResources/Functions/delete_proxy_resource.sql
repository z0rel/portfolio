-- Function: delete_proxy_resource(bigint)

DROP FUNCTION delete_proxy_resource(bigint);

CREATE OR REPLACE FUNCTION delete_proxy_resource(
	IN resource_id_in bigint,

	OUT resource_name_out varchar,
	OUT resource_description_out varchar,
	OUT internal_address_out varchar,
	OUT external_address_out varchar,
	OUT auth_mode_out tls_type)
  AS
$BODY$
declare
	subject_id_ bigint;
	rid_ int;
begin
	if resource_id_in is null then
		return;
	end if;

	-- Получаем ID текущего сертификата пользователя
	select into subject_id_ get_current_user_cert_id();

	if not exists (select * from proxy_resources where resource_id = resource_id_in) then
		execute 'select add_last_error(3);'; -- Удаляемый объект не существует.
		return;
	end if;

	if not check_permission_to_object_type(subject_id_, 'proxy_resource', null, 'w') then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	-- удаляем проксируемый ресурс
	delete from proxy_resources PR where PR.resource_id = resource_id_in
	returning PR.resource_name, PR.resource_description, PR.internal_address, PR.external_address, PR.auth_mode
	into resource_name_out, resource_description_out, internal_address_out, external_address_out, auth_mode_out;

	-- ngingxdb_proxy
	delete from nginxdb_proxy_resources NDBR where NDBR.external_addr = external_address_out returning NDBR.rid into rid_;
	delete from nginxdb_proxy_permissions NDBP where NDBP.rid = rid_;

	-- меняем версию БД
	update db_version set dbversion = dbversion + 1;

end;
$BODY$
	LANGUAGE plpgsql VOLATILE
	COST 100;
ALTER FUNCTION delete_proxy_resource(bigint)
	OWNER TO postgres;
COMMENT ON FUNCTION delete_proxy_resource(bigint) IS 'Выполняет удаление проксируемого ресурса';
