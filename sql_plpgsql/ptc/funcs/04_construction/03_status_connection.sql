create or replace function api_construction__status_connection() returns trigger as
$$
-- declare
    -- new api_construction%ROWTYPE;
begin
    if tg_op = 'UPDATE' or tg_op = 'INSERT' then
        if exists(
                select 1
                  from api_construction_tech_problem
                       inner join api_techproblems a on api_construction_tech_problem.techproblems_id = a.id
                 where api_construction_tech_problem.construction_id = new.id
                     and a.title ilike '%нет напряжения%'
                    or a.title ilike '%не работают все диоды%'
            )
        then
            new.status_connection := false;
        end if;
        return new;
    elsif (tg_op = 'DELETE') then
        return old;
    end if;
    return new;
end;
$$ language plpgsql;