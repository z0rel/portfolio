-- Обновление числа распределений на монтаж элемента бронирования
drop trigger if exists api_mounting__count_reservations_distributed_to_mountings on api_mounting restrict;
create trigger api_mounting__count_reservations_distributed_to_mountings
    before update or insert
    on api_mounting
    for each row
execute procedure api_mounting__count_reservations_distributed_to_mountings();

-- Запрет установки даты монтажа позднее даты демонтажа
drop trigger if exists api_mounting__prevent_incompatible_terms on api_mounting restrict;
create trigger api_mounting__prevent_incompatible_terms
    before update or insert
    on api_mounting
    for each row
execute procedure api_mounting__prevent_incompatible_terms();