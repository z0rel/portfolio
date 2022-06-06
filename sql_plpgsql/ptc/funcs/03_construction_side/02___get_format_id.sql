-- Получить id формата конструкции по id рекламной стороны
create or replace function api_construction_side__get_format_id(advertising_side_id int) returns bigint as
$$
begin
    if advertising_side_id is NULL then
        return NULL;
    end if;

    return (select format_id
            from api_side
                     join
                 api_advertisingside on api_side.id = api_advertisingside.side_id
            where api_advertisingside.id = advertising_side_id);
end;
$$ language plpgsql;
