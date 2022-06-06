-- Получить id района по id местоположения
create or replace function api_location____query_location_district_id(
    postcode_id bigint
) returns bigint as
$$
begin
    if postcode_id is null then
        return null;
    end if;
    return (select api_postcode.district_id
            from api_postcode
            where api_postcode.id = postcode_id);
end;
$$ language plpgsql;


-- При обновлении почтового кода в местоположении, если вместе с ним обновляется район - переиндексировать
-- номера конструкций
create or replace function api_location__num_in_district() returns trigger as
$$
declare
    -- new api_location%ROWTYPE;
    -- old api_location%ROWTYPE;

    district_id_old bigint;
    district_id_new bigint;
begin
    if (tg_op = 'UPDATE') then
        if left_notnull_bigint(old.postcode_id,  new.postcode_id) then
            -- обнулить все номера конструкций для этого местоположения
            update api_construction set num_in_district = null where location_id = new.id;
        elsif right_neq_bigint(old.postcode_id, new.postcode_id) then
            district_id_old := api_location____query_location_district_id(old.postcode_id);
            district_id_new := api_location____query_location_district_id(new.postcode_id);
            if left_notnull_bigint(district_id_old,  district_id_new) then
                -- обнулить все номера конструкций для этого местоположения
                update api_construction set num_in_district = null where location_id = new.id;
            elsif right_neq_bigint(district_id_old, district_id_new) then
                -- переиндексировать все номера конструкций для этого местоположения
                update api_construction
                  set num_in_district = api_construction____update_api_lastserialnumber(district_id_new)
                 where location_id = new.id;
            end if;
        end if;
        return new;
    elsif (tg_op = 'DELETE') then
        return old;
    end if;
    return new;
end;
$$ language plpgsql;
