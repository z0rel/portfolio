-- Function: add_proxy_resource(varchar, varchar, varchar, varchar, tls_type)

DROP FUNCTION add_proxy_resource(varchar, varchar, varchar, varchar, tls_type);

CREATE OR REPLACE FUNCTION add_proxy_resource(
		IN name_in varchar,
		IN description_in varchar,
		IN internal_address_in varchar,
		IN external_address_in varchar,
		IN auth_mode_in tls_type,

		OUT resource_id_out bigint) AS
$BODY$
declare
	subject_id_ bigint;
	res_id_ bigint;
begin
		-- Получаем ID текущего сертификата пользователя
	select into subject_id_ get_current_user_cert_id();

	if not check_permission_to_object_type(subject_id_, 'proxy_resource', null, 'w')
	then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	if (name_in is null) or (internal_address_in is null) or (external_address_in is null)
	then
		execute 'select add_last_error(-6);'; --NULL недопустим
		return;
	end if;

	if char_length(name_in) = 0 or char_length(internal_address_in) = 0 or char_length(external_address_in) = 0
	then
		execute 'select add_last_error(-16);'; --Пустой аргумент недопустим
		return;
	end if;

	if exists (select PR.resource_name from proxy_resources PR where PR.resource_name = name_in)
	then
		execute 'select add_last_error(-17);'; --Ресурс с заданным именем уже существует
		return;
	end if;

	if exists (select PR.internal_address from proxy_resources PR where PR.internal_address = internal_address_in)
	then
		execute 'select add_last_error(-18);'; --Внутренний адрес ресурса уже используется
		return;
	end if;

	if  exists (select PR.external_address from proxy_resources PR where PR.external_address = external_address_in)
	then
		execute 'select add_last_error(-19);'; --Внешний адрес ресурса уже используется
		return;
	end if;

	insert into proxy_resources(resource_name, resource_description, internal_address, external_address, auth_mode)
	values(name_in, description_in, internal_address_in, external_address_in, auth_mode_in)
	returning resource_id into resource_id_out;

	-- ngingxdb_proxy
	if (auth_mode_in = 'twoSideTLS')
	then
		insert into nginxdb_proxy_resources(external_addr) values (external_address_in);
	end if;

end;
$BODY$
	LANGUAGE plpgsql VOLATILE
	COST 100;
ALTER FUNCTION add_proxy_resource(varchar, varchar, varchar, varchar, tls_type)
	OWNER TO postgres;
	COMMENT ON FUNCTION add_proxy_resource(varchar, varchar, varchar, varchar, tls_type) IS 'Добавляет новый проксируемый ресурс';
