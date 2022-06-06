-- function add_roles_permission_events()

drop function add_roles_permission_events();

create or replace function add_roles_permission_events()
returns void as
$$
declare
     event_ record;
     role_ record;
begin
     for event_ in select * from ref_types_events RTE loop
          for role_ in select * from roles R loop

               if not exists(select *
                             from permission_role_event PRE
                             where PRE.role_id = role_.role_id
                                   and PRE.ev_type_id = event_.event_type_id)
               then
                    if (role_.string_id = any(event_.allowed_to_roles)) then
                        insert into permission_role_event(role_id, ev_type_id)
                            values(role_.role_id, event_.event_type_id);
                    end if;
               end if;

          end loop;
     end loop;
end;
$$
  language plpgsql volatile
  cost 100;
alter function add_roles_permission_events()
  owner to postgres;
comment on function add_roles_permission_events() is 'Внутренняя функция. Заполнение таблицы разрешений ролей на чтение событий.';

select add_roles_permission_events();