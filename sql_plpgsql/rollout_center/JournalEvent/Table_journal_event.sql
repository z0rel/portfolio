--  table journal_event

drop table journal_event cascade;
drop sequence journal_event_id_seq;

CREATE SEQUENCE journal_event_id_seq
  INCREMENT 1
  MINVALUE 0
  MAXVALUE 9223372036854775807
  START 0
  CACHE 1;

create table journal_event(
id bigint NOT NULL DEFAULT nextval('journal_event_id_seq'::regclass),
reg_time timestamp with time zone default now(),
event_type_id bigint,
subject_id bigint,
subject_role_group role_group, -- группа ролей 'admin'/'user', можно указать для субъекта, который является пользовательским сертификатом
object_id bigint,
object_role_group role_group, -- группа ролей 'admin'/'user', можно указать для объекта, который является пользовательским сертификатом
result boolean,
args varchar[],
constraint pk_journal_event_id primary key (id)
);