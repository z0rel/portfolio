DROP TYPE role_group CASCADE;
CREATE TYPE role_group AS ENUM ('admin', 'user');

-- Последовательности в соответствии со справочной таблицей объектов

-- min value: (sid - 1) * 2 ^ 32
-- max value: (sid * 2 ^ 32) – 1.

drop SEQUENCE users_certificates_id_seq cascade;

CREATE SEQUENCE users_certificates_id_seq
  INCREMENT 1
  MINVALUE 0
  MAXVALUE 4294967295
  START 0
  CACHE 1;
ALTER TABLE users_certificates_id_seq
  OWNER TO postgres;


drop SEQUENCE roles_id_seq cascade;

CREATE SEQUENCE roles_id_seq
  INCREMENT 1
  MINVALUE 4294967296
  MAXVALUE 8589934591
  START 4294967296
  CACHE 1;
ALTER TABLE roles_id_seq
  OWNER TO postgres;


drop SEQUENCE proxy_resources_id_seq cascade;

CREATE SEQUENCE proxy_resources_id_seq
  INCREMENT 1
  MINVALUE 8589934592
  MAXVALUE 12884901887
  START 8589934592
  CACHE 1;
ALTER TABLE proxy_resources_id_seq
  OWNER TO postgres;


drop SEQUENCE event_type_id_seq cascade;

CREATE SEQUENCE event_type_id_seq
  INCREMENT 1
  MINVALUE 12884901888
  MAXVALUE 17179869183
  START 12884901888
  CACHE 1;
ALTER TABLE event_type_id_seq
  OWNER TO postgres;


drop SEQUENCE system_objects_id_seq cascade;

CREATE SEQUENCE system_objects_id_seq
  INCREMENT 1
  MINVALUE 17179869184
  MAXVALUE 21474836479
  START 17179869184
  CACHE 1;
ALTER TABLE system_objects_id_seq
  OWNER TO postgres;


drop SEQUENCE root_certificate_id_seq cascade;

CREATE SEQUENCE root_certificate_id_seq
  INCREMENT 1
  MINVALUE 21474836480
  MAXVALUE 25769803775
  START 21474836480
  CACHE 1;
ALTER TABLE root_certificate_id_seq
  OWNER TO postgres;


drop SEQUENCE backup_id_seq cascade;

CREATE SEQUENCE backup_id_seq
  INCREMENT 1
  MINVALUE 25769803776
  MAXVALUE 30064771071
  START 25769803776
  CACHE 1;
ALTER TABLE backup_id_seq
  OWNER TO postgres;