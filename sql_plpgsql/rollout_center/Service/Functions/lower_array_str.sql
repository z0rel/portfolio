-- function lower_array_str(varchar[]);

drop function lower_array_str(varchar[]);

create or replace function lower_array_str(str_arr_in varchar[]) returns text[] as
$$
 select array_agg(lower(V)) FROM unnest($1) V;
$$ language sql;