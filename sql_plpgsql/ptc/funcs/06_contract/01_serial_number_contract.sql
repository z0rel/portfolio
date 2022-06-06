-- Нумерация контракта в месяце
create or replace function api_contract__serial_number() returns trigger as
$$
declare
    num         bigint;
    year_unique smallint;

begin
    if (tg_op = 'INSERT') then
        if new.start is null then
            raise notice 'field start cannot be empty';
            return null;
        end if;
        year_unique = (CAST(Extract(year from new.start::date) as smallint));

        insert into api_lastserialnumbercontract (year, number) values (year_unique, 1)
        on conflict (year) do update set number = api_lastserialnumbercontract.number + 1
        returning number into num;
        new.serial_number := format('№Д-%s/%s', num, year_unique);
        return new;
    elsif (tg_op = 'DELETE') then
        return old;
    end if;
    return new;
end;
$$ language plpgsql;