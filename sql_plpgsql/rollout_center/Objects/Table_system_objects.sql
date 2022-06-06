-- table system_objects

drop table system_objects;

create table system_objects(
obj_id bigint default nextval('system_objects_id_seq'::regclass),
local_name varchar,
display_name varchar
);

insert into system_objects(local_name, display_name)
     values('system', 'TLS Gateway');

insert into system_objects(local_name, display_name)
     values('license', 'License');