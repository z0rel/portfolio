-- Обновить максимальный номер конструкции в районе и вернуть его
drop function if exists api_construction____update_api_lastserialnumber(integer) cascade;

create or replace function api_construction____update_api_lastserialnumber(
    arg_district_id bigint
) returns bigint as
$$
declare
    num bigint;
begin
    if arg_district_id is null then
        return null;
    end if;

    insert into api_lastserialnumber (number, district_id) values (1, arg_district_id)
    on conflict (district_id) do update set number = api_lastserialnumber.number + 1
    returning number into num;
    return num;
end;
$$ language plpgsql;
