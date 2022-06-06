create or replace function api_project__num_in_year_project() returns trigger as
$$
declare
    num         bigint;
    year_unique smallint;

begin
    if (tg_op = 'INSERT' or tg_op = 'UPDATE') then
        if new.created_at is null or new.num_in_year is not null then
            return new;
        end if;
        year_unique = (CAST(Extract(year from new.created_at::date) as smallint));

        insert into api_lastnumyearproject (year, number) values (year_unique, 1)
        on conflict (year) do update set number = api_lastnumyearproject.number + 1
        returning number into num;

        new.num_in_year := num;

        if new.code is null then
            new.code := format('%s-%s', year_unique, new.num_in_year);
        end if;
        return new;
    elsif (tg_op = 'DELETE') then
        return old;
    end if;
end;
$$ language plpgsql;