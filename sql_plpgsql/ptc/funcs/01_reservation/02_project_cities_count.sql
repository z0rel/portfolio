-- Выполнить подсчет числа городов в проекте по данным бронирования
create or replace function api_reservation__project_cities_count() returns trigger as
$$
declare
    -- new api_reservation%ROWTYPE;
    -- old api_reservation%ROWTYPE;
    is_saled_old boolean            := false;
    is_saled_new boolean            := false;
    is_distributed_old boolean      := false;
    is_distributed_new boolean      := false;
    change_total_value bigint       := 0;
    change_saled_value bigint       := 0;
    change_distributed_value bigint := 0;
    saled_type_id bigint            := null;
begin
    saled_type_id = (select id from api_reservationtype where ikey = 4); -- 4 = SALED

    if (tg_op = 'INSERT') then
        is_saled_new := new.reservation_type_id is not null and saled_type_id is not null and
                        new.reservation_type_id = saled_type_id;
        is_distributed_new := new.distributed_to_mounting is not null and
                              new.distributed_to_mounting > 0; -- новая запись была распределена на монтаж => увеличить ее счетчик
        if is_saled_new then
            change_saled_value := 1;
        end if;
        if is_distributed_new then
            change_distributed_value := 1;
        end if;

        call api_reservation__project_cities_update_count_value(
                new.construction_side_id, -- INSERT increment new
                new.project_id,
                1,
                change_saled_value,
                change_distributed_value,
                is_saled_new,
                is_distributed_new,
                false
            );
        return new; -- Идентификатор конструкции пуст, не делать ничего
    elsif (tg_op = 'UPDATE') then
        is_saled_old := old.reservation_type_id is not null and saled_type_id is not null and
                        old.reservation_type_id = saled_type_id;
        is_saled_new := new.reservation_type_id is not null and saled_type_id is not null and
                        new.reservation_type_id = saled_type_id;
        is_distributed_old := old.distributed_to_mounting is not null and
                              old.distributed_to_mounting > 0; -- старая запись была распределена на монтаж => уменьшить ее счетчик
        is_distributed_new := new.distributed_to_mounting is not null and
                              new.distributed_to_mounting > 0; -- новая запись была распределена на монтаж => увеличить ее счетчик

        if (old.construction_side_id <> new.construction_side_id or old.project_id <> new.project_id) then
            change_total_value := 1;
        end if;

        if is_saled_old != is_saled_new then
            change_saled_value := 1;
        end if;
        if is_distributed_old != is_distributed_new then
            change_distributed_value := 1;
        end if;

        -- Если старый и новый проект и сторона те же и состояния распределенности и проданности не изменились - не делать ничего
        if old.construction_side_id is not null
            and old.project_id is not null
            and new.construction_side_id is not null
            and new.project_id is not null
            and old.construction_side_id = new.construction_side_id
            and old.project_id = new.project_id
            and is_distributed_old = is_distributed_new
            and is_saled_old = is_saled_new
        then
            return new;
        end if;

        call api_reservation__project_cities_update_count_value(
                old.construction_side_id, -- UPDATE 1: decrement old
                old.project_id,
                -change_total_value,
                -change_saled_value,
                -change_distributed_value,
                is_saled_old, -- Если в старой записи для старой стороны был статус "продано" - декрементировать число проданных
                is_distributed_old,
                false
            );
        call api_reservation__project_cities_update_count_value(
                new.construction_side_id, -- UPDATE 2: increment new
                new.project_id,
                change_total_value,
                change_saled_value,
                change_distributed_value,
                is_saled_new, -- Если в новой записи для новой стороны есть статус "продано" - инкрементировать число проданных
                is_distributed_new,
                false
            );
        return new;
    elsif (tg_op = 'DELETE') then
        is_saled_old := old.reservation_type_id is not null and saled_type_id is not null and
                        old.reservation_type_id = saled_type_id;
        is_distributed_old := old.distributed_to_mounting is not null and
                              old.distributed_to_mounting > 0; -- старая запись была распределена на монтаж => уменьшить ее счетчик
        if is_saled_old then
            change_saled_value := 1;
        end if;
        if is_distributed_old then
            change_distributed_value := 1;
        end if;

        call api_reservation__project_cities_update_count_value(
                old.construction_side_id, -- DELETE decrement old
                old.project_id,
                -1,
                -change_saled_value,
                -change_distributed_value,
                is_saled_old,
                is_distributed_old,
                true
            );
        return old;
    end if;
    return new;
end;
$$ language plpgsql;


