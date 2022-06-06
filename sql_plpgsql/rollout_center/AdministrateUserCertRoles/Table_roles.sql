-- Table: roles

drop type permission_type cascade;
create type permission_type AS ENUM ('r', 'w');

DROP TABLE roles CASCADE;
CREATE TABLE roles
(
  role_id bigint NOT NULL DEFAULT nextval('roles_id_seq'::regclass),
  string_id character varying NOT NULL,
  group_type role_group NOT NULL,
  display_name character varying,
  comment_field character varying,
  CONSTRAINT pk_role_id PRIMARY KEY (role_id)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE roles
  OWNER TO postgres;

insert into roles (role_id, string_id, group_type, display_name, comment_field)
values((select((2 - 1) * 2 ^ 32) + 1), 'seca', 'admin', 'Администратор безопасности', 'Управляет списком доверенных УЦ, транспортными ключами TLS Gateway и полномочиями учетных записей, имеющих доступ к административному интерфейсу.');

insert into roles (role_id, string_id, group_type, display_name, comment_field)
values((select((2 - 1) * 2 ^ 32) + 2), 'a', 'admin', 'Администратор', 'Управляет доступом пользователей к ресурсами и списком проксируемых ресурсов.');

insert into roles (role_id, string_id, group_type, display_name, comment_field)
values((select((2 - 1) * 2 ^ 32) + 3), 'aud', 'admin', 'Аудитор', 'Просмотр и архивация журнала аудита.');

insert into roles (role_id, string_id, group_type, display_name, comment_field)
values((select((2 - 1) * 2 ^ 32) + 4), 'u', 'user', 'Пользователь <Р>', 'Доступа к административному интерфейсу не имеет, использует TLS Gateway как прокси для доступа к ресурсу <Р>.');

insert into roles (role_id, string_id, group_type, display_name, comment_field)
values((select((2 - 1) * 2 ^ 32) + 5), 'bu', 'user', 'Блокированный пользователь системы', 'Достук к системе запрещен.');
