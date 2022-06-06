-- function function set_tlsgtw_option(varchar, varchar, varchar, bigint)

drop function set_tlsgtw_option(varchar, varchar, varchar, bigint);

CREATE OR REPLACE FUNCTION set_tlsgtw_option(sectionname_in varchar, namevalue_in varchar, new_str_value_in varchar, new_int_value_in bigint)
  RETURNS table(
  strvalue varchar,
  intvalue bigint)
AS
$BODY$
begin
  if sectionname_in is null or namevalue_in is null then
    execute 'select add_last_error(-6);'; -- NULL недопустим.
    return ;
  end if;

  if exists(select *
            from tlsgtw_options TGO
            where TGO.namesection = sectionname_in
                  and TGO.nameoption = namevalue_in
                  and TGO.editable = false) then
    execute 'select add_last_error(-14);'; -- Редактирование объекта не допускается.
    return ;
  end if;

  if exists(select *
            from tlsgtw_options TGO
            where TGO.namesection = sectionname_in
                  and TGO.nameoption = namevalue_in ) then
    update tlsgtw_options TGO
      set strvalue = coalesce(new_str_value_in, TGO.strvalue),
        intvalue = coalesce(new_int_value_in, TGO.intvalue)
    where TGO.namesection = sectionname_in and TGO.nameoption = namevalue_in;
  else
    insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
    VALUES (sectionname_in, namevalue_in, new_int_value_in, new_str_value_in, true);
  end if;

  return query select TGO.strvalue, TGO.intvalue
               from tlsgtw_options TGO
               where TGO.namesection = sectionname_in and TGO.nameoption = namevalue_in;
end;
$BODY$
LANGUAGE plpgsql VOLATILE;