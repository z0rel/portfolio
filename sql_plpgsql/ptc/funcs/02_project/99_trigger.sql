-- Нумерация проекта в году
drop trigger if exists api_project__num_in_year_project on api_project restrict;
create trigger api_project__num_in_year_project
    before update or insert
    on api_project
    for each row
execute procedure api_project__num_in_year_project();
