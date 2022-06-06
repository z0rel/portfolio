-- table current_msg_id_entry

drop table current_msg_id_entry cascade;

create table current_msg_id_entry(
msg_id bigint,
constraint current_msg_id_entry_id_fkey foreign key(msg_id)
references journal_event(id) match full
);