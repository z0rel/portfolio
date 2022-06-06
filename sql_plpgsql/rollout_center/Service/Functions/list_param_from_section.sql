drop function list_param_from_section(sectionname varchar);

CREATE OR REPLACE FUNCTION list_param_from_section(sectionname varchar)
  RETURNS table(paramname varchar, int_value bigint, string_value varchar)
AS
$$
begin
   return query select nameoption as paramname, intvalue as int_value, strvalue as string_value
   from tlsgtw_options
   where namesection = sectionname;
end;
$$
LANGUAGE plpgsql VOLATILE;