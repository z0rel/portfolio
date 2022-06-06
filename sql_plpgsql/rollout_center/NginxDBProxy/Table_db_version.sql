-- table current_msg_id_entry

drop table db_version cascade;

create table db_version
(
dbversion bigint
);

insert into db_version (dbversion) values (0);