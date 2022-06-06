-- Контроль целостности совпадения формата конструкции и стороны конструкции
drop trigger if exists api_construction_side__integrity_format on api_constructionside restrict;
create trigger api_construction_side__integrity_format
    before update or insert
    on api_constructionside
    for each row
execute procedure api_construction_side__integrity_format();


drop trigger if exists api_construction_side__count_owner_format on api_constructionside restrict;
create trigger api_construction_side__count_owner_format
    before update or insert or delete
    on api_constructionside
    for each row
execute procedure api_construction_side__count_owner_format();

-- выводить статус недоступности конструкции как "недоступна", если все ее стороны недоступны
drop trigger if exists api_construction_side__status_availability on api_constructionside restrict;
create trigger api_construction_side__status_availability
    after update or insert
    on api_constructionside
    for each row
execute procedure api_construction_side__status_availability();
