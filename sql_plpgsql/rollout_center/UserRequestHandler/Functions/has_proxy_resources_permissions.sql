-- Function: get_proxy_resource_permission(varchar)

CREATE OR REPLACE FUNCTION get_proxy_resource_permission(resource_addr_ varchar)
  RETURNS boolean AS
$BODY$
DECLARE
  ret_ boolean;
  user_cert_id_ bigint;
BEGIN
  -- берём id сертификата того пользователя, то имени которого проверяем доступ
  select into user_cert_id_ get_current_user_cert_id();
  -- берём данные о доступе из БД
  select urae.access_allowed into ret_
  from user_resources_access_entries urae,
       proxy_resources pr
  where urae.user_cert_id=user_cert_id_ and
        urae.resource_id=pr.resource_id and
        pr.external_address=resource_addr_
  limit 1;
  -- если в БД нет данных, то и доступа тоже нет
  if FOUND then
    return ret_;
  else
    return false;
  end if;
END;
$BODY$
  LANGUAGE plpgsql;
ALTER FUNCTION get_proxy_resource_permission(varchar)
OWNER TO postgres;
COMMENT ON FUNCTION get_proxy_resource_permission(varchar)
  IS 'Возвращает true, если есть разрешение на доступ к ресурсу по внешнему адресу resource_addr_. Иначе возвращает false.';