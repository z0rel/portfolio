-- Нумерация контракта в месяце
drop trigger if exists api_contract__serial_number on api_contract restrict;
create trigger api_contract__serial_number
    before update or insert
    on api_contract
    for each row
execute procedure api_contract__serial_number();
