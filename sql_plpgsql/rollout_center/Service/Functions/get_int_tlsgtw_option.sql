-- function get_int_tlsgtw_option(varchar, varchar)

drop function get_int_tlsgtw_option(varchar, varchar);

CREATE OR REPLACE FUNCTION get_int_tlsgtw_option(IN sectionname varchar, IN namevalue varchar, OUT value bigint)
AS
$BODY$
begin
  if not exists(select * from tlsgtw_options TGO where TGO.namesection = sectionname and TGO.nameoption = namevalue) then
    execute 'select add_last_error(-2);'; -- Объекта с нужным именем и типом нет.
    return;
  end if;

  select into value TGO.intvalue
               from tlsgtw_options TGO
               where TGO.namesection = sectionname and TGO.nameoption = namevalue limit 1;

  return;
end;
$BODY$
LANGUAGE plpgsql VOLATILE;