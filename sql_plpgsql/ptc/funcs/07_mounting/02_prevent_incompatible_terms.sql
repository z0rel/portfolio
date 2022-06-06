-- Запрет установки даты монтажа позднее даты демонтажа
create or replace function api_mounting__prevent_incompatible_terms() returns trigger as
$$
begin
    if NEW.start_mounting > NEW.end_mounting then
        raise EXCEPTION 'Дата монтажа не может быть позднее чем дата демонтажа.';
    end if;
    return NEW;
end;
$$ language plpgsql;
