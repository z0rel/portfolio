-- Проверка - семейство конструкции совпадает с семейством местоположения на котором она установлена

create or replace function api_construction__integrity_family() returns trigger as
$$
declare
    -- new api_construction%ROWTYPE;
    -- old api_construction%ROWTYPE;

    fmc_join_location integer;
    fmc_join_format   integer;
    is_integrity      bool;
begin
    if (tg_op = 'UPDATE') then
        is_integrity = (select apfc1.id = apfc2.id
                          from api_construction
                               left join api_location on new.location_id = api_location.id
                               left join api_familyconstruction apfc1 on api_location.family_construction_id = apfc1.id
                               left join api_modelconstruction am on api_construction.model_id = am.id
                               left join api_underfamilyconstruction
                                         on am.underfamily_id = api_underfamilyconstruction.id
                               left join api_familyconstruction apfc2
                                         on api_underfamilyconstruction.family_id = apfc2.id
                         where api_construction.id = new.id);

        if is_integrity = false then
            raise exception 'api_familyconstruction_id from api_location must match api_familyconstruction_id in api_construction';
        end if;
        return new;
    elsif (tg_op = 'INSERT') then
        fmc_join_location = (select api_familyconstruction.id
                               from api_location
                                    left join api_familyconstruction
                                              on api_location.family_construction_id = api_familyconstruction.id
                              where api_location.id = new.location_id);

        fmc_join_format = (select api_familyconstruction.id
                             from api_modelconstruction
                                  left join api_underfamilyconstruction
                                            on api_modelconstruction.underfamily_id = api_underfamilyconstruction.id
                                  left join api_familyconstruction
                                            on api_underfamilyconstruction.id = api_familyconstruction.id
                            where api_modelconstruction.id = new.model_id);

        if fmc_join_location <> fmc_join_format then
            raise exception 'api_familyconstruction_id from api_location must match api_familyconstruction_id in api_construction';
        end if;
        return new;
    elsif (tg_op = 'DELETE') then
        return old;
    end if;
    return new;
end;
$$ language plpgsql;