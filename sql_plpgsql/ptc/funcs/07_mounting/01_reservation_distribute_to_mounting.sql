-- Обновление числа распределений на монтаж элемента бронирования
create or replace function api_mounting__count_reservations_distributed_to_mountings() returns trigger as
$$
begin
    if (tg_op = 'INSERT') then
        if new.reservation_id is not null then
            update api_reservation
               set distributed_to_mounting = distributed_to_mounting + 1
             where api_reservation.id = new.reservation_id;
        end if;
        return new;
    elsif (tg_op = 'UPDATE') then
        if new.reservation_id is not null and old.reservation_id is null or
           old.reservation_id <> new.reservation_id then
            update api_reservation
               set distributed_to_mounting = distributed_to_mounting + 1
             where api_reservation.id = new.reservation_id;

            if old.reservation_id is not null then
                update api_reservation
                   set distributed_to_mounting = distributed_to_mounting - 1
                 where api_reservation.id = old.reservation_id;
            end if;
        end if;
        return new;
    elsif (tg_op = 'DELETE') then
        if old.reservation_id is not null then
            update api_reservation
               set distributed_to_mounting = distributed_to_mounting - 1
             where api_reservation.id = old.reservation_id;
        end if;
        return old;
    end if;
    return new;
end;
$$ language plpgsql;
