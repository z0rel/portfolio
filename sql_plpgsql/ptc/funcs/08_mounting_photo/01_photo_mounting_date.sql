-- Обновление даты фотографии монтажа
create or replace function api_mounting_photo__update_photo_date() returns trigger as
$$
begin
    if (tg_op = 'INSERT') then
        if new.photo is not null then
            new.date := clock_timestamp();
        end if;
        return new;
    elsif (tg_op = 'UPDATE') then
        if new.photo != old.photo and new.photo is not null then
            new.date := clock_timestamp();
        end if;
        return new;
    elsif (tg_op = 'DELETE') then
        return old;
    end if;
    return new;
end;
$$ language plpgsql;
