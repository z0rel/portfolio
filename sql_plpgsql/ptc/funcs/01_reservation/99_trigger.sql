-- Обновление числа городов для проекта
drop trigger if exists api_reservation__project_cities_count on api_reservation restrict;
create trigger api_reservation__project_cities_count
    before insert or update or delete
    on api_reservation
    for each row
execute procedure api_reservation__project_cities_count();
