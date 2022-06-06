-- function check_permission_to_object(bigint, bigint, role_group, permission_type)

drop function check_permission_to_object(bigint, bigint, role_group, permission_type);

create or replace function check_permission_to_object(user_cert_id_in bigint, object_id_in bigint, obj_role_group role_group, permission permission_type) -- 'r', 'w'
returns boolean as
$$
declare
  type_object_ varchar;
  object_name_ varchar;
begin
  if (user_cert_id_in is null or object_id_in is null) then
    execute 'select add_last_error(-6);'; -- NULL недопустим.
    return false;
  end if;

  select get_type_object_by_id(object_id_in, obj_role_group) into type_object_;
  select get_string_name_by_id(object_id_in) into object_name_;

  if (type_object_ is null) then
    execute 'select add_last_error(-8);'; -- Объекта с данным идентификатором нет.
    return false;
  end if;

  if (type_object_ = 'event_type') then
        return check_permission_to_event(user_cert_id_in, object_id_in);
  end if;

  return check_permission_to_object_type(user_cert_id_in, type_object_, object_name_, permission);
end;
$$
language plpgsql;