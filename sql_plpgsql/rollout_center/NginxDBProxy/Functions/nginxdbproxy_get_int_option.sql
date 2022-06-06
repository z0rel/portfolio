-- function nginxdbproxy_get_int_option(varchar, varchar)

drop function nginxdbproxy_get_int_option(varchar, varchar);

CREATE OR REPLACE FUNCTION nginxdbproxy_get_int_option(IN sectionname varchar, IN namevalue varchar, OUT value bigint)
AS
$BODY$
  select TGO.intvalue
               from tlsgtw_options TGO
               where TGO.namesection = sectionname and TGO.nameoption = namevalue limit 1;
$BODY$
  LANGUAGE sql;
ALTER FUNCTION nginxdbproxy_get_int_option(varchar, varchar)
OWNER TO postgres;
COMMENT ON FUNCTION nginxdbproxy_get_int_option(varchar, varchar)
  IS 'Получение опции namevalue из секции sectionname. ';