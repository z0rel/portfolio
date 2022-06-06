-- Function: nginxdb_proxy_get_proxy_resource_permission(varchar, varchar, varchar)

drop function nginxdb_proxy_get_proxy_resource_permission(varchar, varchar, varchar);

CREATE OR REPLACE FUNCTION nginxdb_proxy_get_proxy_resource_permission(
	IN serial_num_in varchar,
	IN issuer_dn_in varchar,
	IN resource_add_in varchar,
	OUT access_allowed boolean,
	OUT db_resources_permissions_version bigint) AS
$BODY$
declare
	iid_ int;
	rid_ int;
begin

	select dbversion from db_version into db_resources_permissions_version;

	select I.iid from nginxdb_proxy_issuer I where I.issuer_dn = issuer_dn_in into iid_;
	if (iid_ is null)
	then
		access_allowed := false;
		return;
	end if;

	select R.rid from nginxdb_proxy_resources R where R.external_addr = resource_add_in into rid_;
	if (rid_ is null)
	then
		access_allowed := false;
		return;
	end if;

	select coalesce((select true
								from  nginxdb_proxy_permissions P
								where P.iid = iid_ and
											P.rid = rid_ and
											P.cert_serial_number = serial_num_in
											), false) into access_allowed;

end
$BODY$
	LANGUAGE plpgsql;
ALTER FUNCTION nginxdb_proxy_get_proxy_resource_permission(varchar, varchar, varchar)
OWNER TO postgres;
COMMENT ON FUNCTION nginxdb_proxy_get_proxy_resource_permission(varchar, varchar, varchar)
	IS 'Возвращает true, если есть разрешение на доступ к ресурсу по внешнему адресу resource_add_in. Иначе возвращает false. (На языке sql скорость работы меньше в 4-6 раз).';