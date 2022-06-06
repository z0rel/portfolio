-- table ref_types_events

drop table ref_types_events cascade;

create table ref_types_events(
event_type_id bigint DEFAULT nextval('event_type_id_seq'::regclass),
local_name varchar,
local_name_category varchar,
description varchar,
allowed_to_roles varchar[],
state bigint,
args varchar[],
constraint pk_ref_types_events_id primary key (event_type_id),
constraint ref_types_events_state_fkey foreign key (state)
   references ref_state_system(id) match full
);

-- common event types 1
-- TODO

-- authorize event types 2
insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('authorization_attempt', 'tlsgtw_authorization_ev_type', 'Авторизация в интерфесе управления.', ARRAY['seca','aud'], 1, '{}');

-- certificates access edit event types 4
insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('added_by_admin', 'tlsgtw_access_edit_ev_type', 'Добавление сертификата по команде администратора.', ARRAY['a','aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('added_by_rule', 'tlsgtw_access_edit_ev_type', 'Добавление сертификата в результате срабатывания правила.', ARRAY['a','aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('added_after_request', 'tlsgtw_access_edit_ev_type', 'Добавление сертификата в результате одобрения запроса.', ARRAY['a','aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('cert_removed', 'tlsgtw_access_edit_ev_type', 'Сертификат пользователя удален из системы.', ARRAY['aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('admin_role_added', 'tlsgtw_access_edit_ev_type', 'Сертификату добавлена роль администратора.', ARRAY['seca','aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('admin_role_removed', 'tlsgtw_access_edit_ev_type', 'У сертификата удалена роль администратора.', ARRAY['seca','aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('admin_cert_is_marked_to_remove', 'tlsgtw_access_edit_ev_type', 'Сертификат администратора удален из интерфейса.', ARRAY['seca','aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('cert_blocked', 'tlsgtw_access_edit_ev_type', 'Блокирование сертификата.', ARRAY['a','aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('cert_unblocked', 'tlsgtw_access_edit_ev_type', 'Разблокирование сертификата.', ARRAY['a','aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('user_cert_is_marked_to_remove', 'tlsgtw_access_edit_ev_type', 'Сертификат пользователя удален из интерфейса.', ARRAY['seca','aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('resource_access_allowed', 'tlsgtw_access_edit_ev_type', 'Предоставление доступа пользователя к ресурсу.', ARRAY['a','aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('resource_access_blocked', 'tlsgtw_access_edit_ev_type', 'Блокирование доступа пользователя к ресурсу.', ARRAY['a','aud'], 1, '{}');

-- proxy resources event types 8
insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('resource_added', 'tlsgtw_proxy_res_ev_type', 'Добавление проксируемого ресурса.', ARRAY['a','aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('resource_changed', 'tlsgtw_proxy_res_ev_type', 'Изменение параметров проксируемого ресурса.', ARRAY['a','aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('resource_removed', 'tlsgtw_proxy_res_ev_type', 'Удаление проксируемого ресурса.', ARRAY['a','aud'], 1, '{}');

-- settings event types 16
insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('settings_changed', 'tlsgtw_settings_ev_type', 'Изменение настроек TLS Gateway.', ARRAY['aud'], 1, '{}');

-- keys event types 32
insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('transport_key_activated', 'tlsgtw_keys_ev_type', 'Ввод в действие нового транспортного ключа.', ARRAY['seca','aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('transport_key_expiring', 'tlsgtw_keys_ev_type', 'Истекает срок действия транспортного сертификата.', ARRAY['seca','aud'], 2, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('transport_key_expired', 'tlsgtw_keys_ev_type', 'Истек срок действия транспортного сертификата.', ARRAY['seca','aud'], 4, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('transport_cert_request_downloaded', 'tlsgtw_keys_ev_type', 'Выгружен запрос на сертификат транспортного ключа', ARRAY['seca','aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('transport_cert_uploaded', 'tlsgtw_keys_ev_type', 'Загружен валидный сертификат транспортного ключа.', ARRAY['seca','aud'], 1, '{}');

-- ca event types 64
insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('ca_cert_uploaded', 'tlsgtw_ca_ev_type', 'Добавление корневого сертификата доверенного УЦ.', ARRAY['seca','aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('ca_cert_removed', 'tlsgtw_ca_ev_type', 'Удаление корневого сертификата доверенного УЦ.', ARRAY['seca','aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('ca_cert_expired', 'tlsgtw_ca_ev_type', 'Истек корневой сертификат УЦ', ARRAY['seca','aud'], 4, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('ca_cert_expiring', 'tlsgtw_ca_ev_type', 'Истекает корневой сертификат УЦ.', ARRAY['seca','aud'], 2, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('ca_cert_revoked', 'tlsgtw_ca_ev_type', 'Корневой сертификат УЦ отозван.', ARRAY['seca','aud'], 4, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('ca_crl_changed', 'tlsgtw_ca_ev_type', 'Изменены настройки точки доступа к CRL.', ARRAY['seca','aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('ca_crl_uploaded', 'tlsgtw_ca_ev_type', 'Загружен новый CRL.', ARRAY['seca','aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('ca_crl_expiring', 'tlsgtw_ca_ev_type', 'CRL давно не обновлялся.', ARRAY['seca','aud'], 2, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('ca_crl_expired', 'tlsgtw_ca_ev_type', 'CRL устарел.', ARRAY['seca','aud'], 4, '{}');

-- backup event types 128
insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('backup_created', 'tlsgtw_backup_ev_type', 'Создание резервной копии.', ARRAY['a','aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('backup_downloaded', 'tlsgtw_backup_ev_type', 'Выгрузка резервной копии из системы.', ARRAY['a','aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('backup_uploaded', 'tlsgtw_backup_ev_type', 'Загрузка резервной копии в систему.', ARRAY['a','aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('backup_restored', 'tlsgtw_backup_ev_type', 'Восстановление из резервной копии.', ARRAY['a','aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('backup_removed', 'tlsgtw_backup_ev_type', 'Удаление резервной копии из внутреннего хранилища.', ARRAY['a','aud'], 1, '{}');

-- system event types 256
insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('system_module_started', 'tlsgtw_system_ev_type', 'Запуск системного модуля.', ARRAY['seca'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('system_module_stopped', 'tlsgtw_system_ev_type', 'Плановая остановка системного модуля.', ARRAY['seca'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('system_module_restarted', 'tlsgtw_system_ev_type', 'Перезапуск системного модуля.', ARRAY['seca'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('system_module_reloaded', 'tlsgtw_system_ev_type', 'Применение конфигурации.', ARRAY['seca'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('system_module_unexpectedly_stopped', 'tlsgtw_system_ev_type', 'Остановка системного модуля.', ARRAY['seca'], 4, '{}');

insert into ref_types_events( local_name, local_name_category, description, allowed_to_roles, state, args)
values('system_module_failed_to_start', 'tlsgtw_system_ev_type', 'Не удалось запустить системный модуль.', ARRAY['seca'], 4, '{}');

insert into ref_types_events( local_name, local_name_category, description, allowed_to_roles, state, args)
values('task_state_changed', 'tlsgtw_system_ev_type', 'Изменилось состояние системной задачи.', ARRAY['seca','a','aud'], 1, '{}');

insert into ref_types_events( local_name, local_name_category, description, allowed_to_roles, state, args)
values('change_web_config_succeed', 'tlsgtw_system_ev_type', 'Изменен конфиг nginx', ARRAY['a','aud'], 1, '{}');

insert into ref_types_events( local_name, local_name_category, description, allowed_to_roles, state, args)
values('change_web_config_failed', 'tlsgtw_system_ev_type', 'Попытка изменения конфига nginx закончилась с ошибкой.', ARRAY['a','aud'], 2, '{}');

-- system event types 512
insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('license_expired', 'tlsgtw_license_ev_type', 'Лицензия истекла.', ARRAY['a','aud'], 4, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('license_expiring', 'tlsgtw_license_ev_type', 'Лицензия истекает.', ARRAY['a','aud'], 2, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('license_users_count_expiring', 'tlsgtw_license_ev_type', 'Лицензионные ограничения близки к исчерпанию.', ARRAY['a','aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('license_loaded', 'tlsgtw_license_ev_type', 'Загружена лицензия.', ARRAY['a','aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('license_loaded_not_valid', 'tlsgtw_license_ev_type', 'Была попытка загрузить невалидную лицензию.', ARRAY['a','aud'], 1, '{}');

insert into ref_types_events(local_name, local_name_category, description, allowed_to_roles, state, args)
values('license_incorrect_sw_version', 'tlsgtw_license_ev_type', 'Загруженная лицензия не соответствует версии ПО, установленной на ПАК.', ARRAY['a','aud'], 1, '{}');

