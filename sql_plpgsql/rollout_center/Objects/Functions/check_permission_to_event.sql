-- function check_permission_to_event(bigint,bigint)

DROP FUNCTION check_permission_to_event(bigint,bigint);

create or replace function check_permission_to_event(user_cert_id_in bigint, event_id_in bigint)
returns boolean as
$$
begin
     if (user_cert_id_in = 17179869184) then
          return true; -- tls gateway system identifier
     else
          return exists(
               select *
               from permission_role_event PRE
               where PRE.ev_type_id = event_id_in and
               PRE.role_id in (
                    select URE.role_id
                    from user_roles_entries URE
                    where URE.user_cert_id = user_cert_id_in
               )
          );
     end if;
end;
$$
language plpgsql;