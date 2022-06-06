create or replace function api_construction_side__integrity_format() returns trigger as
$$
declare
    -- new api_constructionside%ROWTYPE;
    -- old api_constructionside%ROWTYPE;

    model_construction      integer;
    model_construction_side integer;
    is_integrity             bool;
begin
    if (tg_op = 'UPDATE') then
        is_integrity = (select afms.id = amc.id
                          from api_constructionside
                               left join api_advertisingside
                                         on api_constructionside.advertising_side_id = api_advertisingside.id
                               left join api_side on api_advertisingside.side_id = api_side.id
                               left join api_format on api_side.format_id = api_format.id
                               left join api_modelconstruction afms on api_format.model_id = afms.id
                               left join api_construction on api_constructionside.construction_id = api_construction.id
                               left join api_modelconstruction amc on api_construction.model_id = amc.id
                         where api_constructionside.id = new.id);

        if is_integrity = false then
            raise exception 'api_format_id from api_constructionside must match api_format_id in api_construction';
        end if;
    elseif (tg_op = 'INSERT') then
        model_construction = (select am.id
                                 from api_construction
                                      left join api_modelconstruction am on api_construction.model_id = am.id
                                where api_construction.id = new.construction_id);

        model_construction_side = (select a.id
                                      from api_advertisingside
                                           left join api_side on api_advertisingside.side_id = api_side.id
                                           left join api_format on api_side.format_id = api_format.id
                                           left join api_modelconstruction a on api_format.model_id = a.id
                                     where api_advertisingside.id = new.advertising_side_id);

        if model_construction <> model_construction_side then
            raise exception 'api_format_id from api_constructionside must match api_format_id in api_construction';
        end if;
        return new;
    elsif (tg_op == 'DELETE') then
        return old;
    end if;
    return new;
end;
$$ language plpgsql;