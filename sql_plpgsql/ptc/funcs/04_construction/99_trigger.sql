-- Контроль целостности семейства конструкции
drop trigger if exists api_construction__integrity_family on api_construction restrict;
create trigger api_construction__integrity_family
    before update or insert
    on api_construction
    for each row
execute procedure api_construction__integrity_family();

-- Нумерация конструкции в районе
drop trigger if exists api_construction__num_in_district on api_construction restrict;
create trigger api_construction__num_in_district
    before update or insert
    on api_construction
    for each row
execute procedure api_construction__num_in_district();

-- Обновление статуса по подключению для конструкции и сторон конструкций
drop trigger if exists api_construction__status_connection on api_construction restrict;
create trigger api_construction__status_connection
    before update or insert
    on api_construction
    for each row
execute procedure api_construction__status_connection();

-- Создание уведомлений для активных пользователей при создании новой конструкции
drop trigger if exists api_construction__create_new_construction_notifications on api_construction restrict;
create trigger api_construction__create_new_construction_notifications
    before update or insert
    on api_construction
    for each row
execute procedure api_construction__create_new_construction_notifications();
