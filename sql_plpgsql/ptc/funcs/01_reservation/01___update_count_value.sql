-- Обновление числа городов в данных проекта при создании бронирования
create or replace procedure api_reservation__project_cities_update_count_value(
    arg_construction_side_id bigint,
    arg_project_id bigint,
    count_value bigint,
    change_saled_value bigint,
    change_distributed_value bigint,
    changed_saled_count boolean, -- Изменился ли статус "Продано"
    changed_distributed_count boolean,
    is_delete boolean) as -- TODO: Изменился ли статус "Отправлено на монтаж
$$
declare
    var_city_id bigint         := null;
    var_saled_cnt bigint       := 0;
    var_distributed_cnt bigint := 0;
begin
    if arg_construction_side_id is null or arg_project_id is null then
        return;
    end if;

    if changed_saled_count then
        var_saled_cnt := change_saled_value;
    end if;

    if changed_distributed_count then
        var_distributed_cnt := change_distributed_value;
    end if;

    select ad.city_id
    into var_city_id
    from api_constructionside
             inner join api_construction ac on api_constructionside.construction_id = ac.id
             inner join api_location     al on ac.location_id = al.id
             inner join api_postcode     ap on al.postcode_id = ap.id
             inner join api_district     ad on ap.district_id = ad.id
    where api_constructionside.id = arg_construction_side_id;

    if var_city_id is null then
        return;
    end if;

    if count_value <> 0 or var_saled_cnt <> 0 or var_distributed_cnt <> 0 then

        if exists(select 1
                  from api_projectcities
                  where api_projectcities.project_id = arg_project_id
                    and api_projectcities.city_id = var_city_id) then
            update api_projectcities
            set count             = count + count_value,
                saled_count       = saled_count + var_saled_cnt,
                distributed_count = distributed_count + var_distributed_cnt
            where city_id = var_city_id
              and project_id = arg_project_id;
        else
            if not is_delete then
                insert into api_projectcities(city_id, project_id, count, saled_count, distributed_count)
                values (var_city_id, arg_project_id, count_value, var_saled_cnt, var_distributed_cnt);
            end if;
        end if;
    end if;
end;
$$ language plpgsql;
