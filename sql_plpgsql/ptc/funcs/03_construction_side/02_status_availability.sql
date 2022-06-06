create or replace function api_construction_side__status_availability() returns trigger as
$$
declare
    result boolean;
begin
    if tg_op = 'UPDATE' or tg_op = 'INSERT' then
        SELECT bool_or(availability_side)
        into result
        from api_constructionside
        where construction_id = new.construction_id;
        if result = FALSE then
            UPDATE api_construction SET status_availability= FALSE WHERE id = new.construction_id;
        else
            UPDATE api_construction SET status_availability= TRUE WHERE id = new.construction_id;
        end if;
    end if;
    return new;
end;
$$ language plpgsql;
