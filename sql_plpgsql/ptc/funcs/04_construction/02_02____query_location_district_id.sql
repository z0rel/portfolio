-- Получить id района по id местоположения
create or replace function api_construction____query_location_district_id(
    location_id bigint
) returns bigint as
$$
declare
    result integer;
begin
    if location_id is NULL then
        return null;
    end if;
    select api_postcode.district_id
    into result
    from api_location
             left join api_postcode on api_location.postcode_id = api_postcode.id
    where api_location.id = location_id;
    return result;
end;
$$ language plpgsql;
