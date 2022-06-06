-- table ref_state_system

drop table ref_state_system cascade;

create table ref_state_system(
id bigint not null default 1,
local_name varchar not null,
display_name varchar not null,
description varchar,
constraint pk_ref_state_system_id primary key (id)
);

insert into ref_state_system(id, local_name, display_name, description)
values(1, 'green', 'Нормальное', 'Нормальное состояние');

insert into ref_state_system(id, local_name, display_name, description)
values(2, 'yellow', 'Есть проблемы', 'Есть проблемы, но немедленного вмешательства не требуется');

insert into ref_state_system(id, local_name, display_name, description)
values(4, 'red', 'Критическое', 'Есть проблемы, которые требуют немедленного вмешательства');