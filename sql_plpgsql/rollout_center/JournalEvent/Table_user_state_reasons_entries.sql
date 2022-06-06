-- table user_state_reasons

drop table user_state_reasons;

create table user_state_reasons(
msg_id bigint,
event_type_id bigint,
object_id bigint,
display_name_object character varying,
when_reg_first timestamp with time zone default now(),
when_reg_last timestamp with time zone default now(),
count bigint default 1,
constraint user_state_reasons_msg_id_fkey foreign key (msg_id)
   references journal_event(id) match full
   on update no action on delete cascade,
constraint user_state_reasons_event_type_id_fkey foreign key (event_type_id)
   references ref_types_events(event_type_id) match full
   on update no action on delete cascade,
constraint uq_user_state_reasons_event_type_id_object_id_key UNIQUE(event_type_id, object_id)
);