DROP FUNCTION auto_rotate_events();

create or replace function auto_rotate_events()
returns table(
     ret_val bigint
) as
$$
declare
     count_events_ bigint;
     event_auto_rotation_green_max_qtty_ bigint;
     event_auto_rotation_keep_in_days_ bigint;
     boundary_ timestamptz;
     maxid_ bigint;
     minid_ bigint;
     yellow_max_ bigint;
     red_max_ bigint;
     qtty_ bigint;
     id_detele_upto_ bigint;
     qtty_to_delete_ bigint;
begin
     select into count_events_ count(*) from journal_event JE;
     select into event_auto_rotation_green_max_qtty_
          coalesce((select get_int_tlsgtw_option('system', 'event_auto_rotation_green_max_qtty')), 10000000);

     if count_events_ < event_auto_rotation_green_max_qtty_ then
          return query select cast(0 as bigint);
          return;
     end if;

     select into event_auto_rotation_keep_in_days_
          coalesce((select get_int_tlsgtw_option('system', 'event_auto_rotation_keep_in_days')), 10);

     execute 'select now() - interval '' $1 days'''
          into boundary_
     using event_auto_rotation_keep_in_days_;

     select into maxid_ JE.id from journal_event JE where JE.reg_time < boundary_ order by JE.reg_time desc limit 1;

     delete from journal_event JE
     where JE.id < maxid_ and not JE.id in(select USR.msg_id from user_state_reasons USR);

     select into yellow_max_ coalesce((select get_int_tlsgtw_option('system', 'event_auto_rotation_yellow_max_qtty')), 90000000);
     select into red_max_ coalesce((select get_int_tlsgtw_option('system', 'event_auto_rotation_red_max_qtty')), 100000000);
     select into minid_ JE.id from journal_event JE order by JE.id asc limit 1;
     select into qtty_ count(*) from journal_event;

     if ( qtty_ < red_max_ ) then
          return query select cast(0 as bigint);
          return;
     end if;

     id_detele_upto_ := minid_ + qtty_ - yellow_max_;

     while true loop
          select into qtty_to_delete_ count(*) from journal_event JE
          where JE.id > minid_
                and JE.id < id_detele_upto_;
          if (qtty_ - qtty_to_delete_) < red_max_ then
               exit; -- выход производится только из цикла while
          end if;

          id_detele_upto_ := id_detele_upto_ + red_max_ - yellow_max_;
     end loop;

     delete from journal_event JE
     where JE.id >= minid_ and JE.id < id_detele_upto_;

     return query select cast(0 as bigint);
end;
$$
language plpgsql;