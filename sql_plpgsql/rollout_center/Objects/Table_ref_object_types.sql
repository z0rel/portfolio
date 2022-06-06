-- Справочная таблица объектов. Содержит информацию о типах объектов и их диапозонов ID
drop table ref_object_types;

create table ref_object_types
(
sid bigint,
local_name varchar(128),
display_name_multi varchar(128),
display_name_single varchar(128),
description varchar(256),
scope_from bigint,
scope_to bigint
);

insert into ref_object_types(sid, local_name, display_name_multi, display_name_single, description, scope_from, scope_to)
values(1, 'certificate', 'Пользовательские сертификаты', 'Пользовательский сертификат', 'Идентификаторы пользовательских (которые могут обладать ролью пользователя и/или администратора TLS Gateway) сертификатов ', 0, 4294967295);

insert into ref_object_types(sid, local_name, display_name_multi, display_name_single, description, scope_from, scope_to)
values(2, 'role', 'Роли', 'Роль', 'Идентификаторы ролей пользователей.', 4294967296, 8589934591);

insert into ref_object_types(sid, local_name, display_name_multi, display_name_single, description, scope_from, scope_to)
values(3, 'proxy_resource', 'Проксируемые ресурсы', 'Проксируемый ресурс', 'Идентификаторы проксируемых ресурсов.', 8589934592, 12884901887);

insert into ref_object_types(sid, local_name, display_name_multi, display_name_single, description, scope_from, scope_to)
values(4, 'event_type', 'Типы событий', 'Тип события', 'Тип события.', 12884901888, 17179869183);

insert into ref_object_types(sid, local_name, display_name_multi, display_name_single, description, scope_from, scope_to)
values(5, 'system', 'Системные объекты', 'Системный объект', 'Системные объекты. Соответствуют различным модулям ПАК TLS Gateway.', 17179869184, 21474836479);

insert into ref_object_types(sid, local_name, display_name_multi, display_name_single, description, scope_from, scope_to)
values(6, 'ca_certificate', 'Доверенные корневые сертификаты', 'Доверенный корневой сертификат', 'Корневые сертификаты УЦ', 21474836480, 25769803775);

insert into ref_object_types(sid, local_name, display_name_multi, display_name_single, description, scope_from, scope_to)
values(7, 'backup_file', 'Файлы резервных копий', 'Файл резервной копии', 'Метаданные файлов резервных копий', 25769803776, 30064771071);