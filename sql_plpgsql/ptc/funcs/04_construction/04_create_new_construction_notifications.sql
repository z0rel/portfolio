create or replace function api_construction__create_new_construction_notifications() returns trigger as
$$
declare
    u_id int;
    ptr_id int;
begin
    for u_id in select id from api_customuser where is_active loop
        insert into api_notification(user_id, topic, read, created_at, updated_at)
        values (u_id, 'Создана новая конструкция.', '0', current_timestamp, current_timestamp) returning id into ptr_id;
        insert into api_constructionnotification (construction_id, notification_ptr_id)
        values (NEW.id, ptr_id);
    end loop;
    return NEW;
end;
$$ language plpgsql;