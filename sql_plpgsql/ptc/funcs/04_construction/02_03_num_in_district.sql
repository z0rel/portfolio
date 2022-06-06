-- Присвоить номер конструкции в районе
create or replace function api_construction__num_in_district() returns trigger as
$$
declare
    district_id__old integer;
    district_id__new integer;
begin
    if new.location_id is null then
        new.num_in_district := null;
        return new;
    end if;
    if (tg_op = 'UPDATE') then
        district_id__new := api_construction____query_location_district_id(new.location_id);

        if district_id__new is null then
            new.num_in_district := null;
            return new;
        end if;

        district_id__old := api_construction____query_location_district_id(old.location_id);
        if right_neq_bigint(district_id__old, district_id__new) then
            new.num_in_district := api_construction____update_api_lastserialnumber(district_id__new);
        end if;
        return new;
    elsif (tg_op = 'INSERT') then
        district_id__new := api_construction____query_location_district_id(new.location_id);
        new.num_in_district := api_construction____update_api_lastserialnumber(district_id__new);
        return new;
    elsif (tg_op = 'DELETE') then
        return old;
    end if;
    return new;
end;
$$ language plpgsql;