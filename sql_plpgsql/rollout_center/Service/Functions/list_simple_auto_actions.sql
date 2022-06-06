-- function list_simple_auto_actions()

drop function list_simple_auto_actions();

create or replace function list_simple_auto_actions()
returns table(
     name varchar,
     pause_in_sec bigint,
     last_execution_ts timestamptz
) as
$$
begin
     return query select AF.name,
                       AF.pause_in_sec,
                       AF.last_execution_ts
                  from auto_functions AF;
end;
$$
language plpgsql;