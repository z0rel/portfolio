-- Обновление даты фотографии монтажа
drop trigger if exists api_mounting_photo__update_photo_date on api_mountingphoto restrict;
create trigger api_mounting_photo__update_photo_date
    before update or insert
    on api_mountingphoto
    for each row
execute procedure api_mounting_photo__update_photo_date();

