-- function auto_process_events(bigint)

drop function auto_process_events(bigint);

create or replace function auto_process_events(max_at_once_in bigint)
returns table(
     ret_val bigint
) as
$$
declare
     max_at_once_ bigint;
     ret_val_ bigint;
begin
     max_at_once_ := coalesce(max_at_once_in, 1);
     ret_val_ := 1;

     while max_at_once_ > 0 and ret_val_ > 0 loop
          execute 'select process_next_event();';
          if (select CMID.msg_id from current_msg_id_entry CMID) <
               (select JE.id from journal_event JE order by JE.id desc limit 1) then
               ret_val_ := 5; -- есть еще события для обработки
          else
               ret_val_ := 0; -- нет больше событий для обработки
          end if;

          max_at_once_ := max_at_once_ - 1;
     end loop;

     return query select ret_val_;
end;
$$
language plpgsql;