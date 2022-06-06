-- function get_string_tlsgtw_option(varchar, varchar)

drop function get_string_tlsgtw_option(varchar,varchar);

CREATE OR REPLACE FUNCTION get_string_tlsgtw_option(IN  sectionname varchar, IN namevalue varchar, OUT value varchar)
AS
$BODY$
begin
    if not exists(select * from tlsgtw_options MCO where MCO.namesection = sectionname and MCO.nameoption = namevalue) then
      execute 'select add_last_error(-2);'; -- Объекта с нужным именем и типом нет.
      return;
    end if;

    select into value strvalue from tlsgtw_options where namesection = sectionname and nameoption = namevalue limit 1;

    return;
end;
$BODY$
LANGUAGE plpgsql VOLATILE;