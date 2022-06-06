-- table ref_categories_events

drop table ref_categories_events;

create table ref_categories_events(
	id bigint NOT NULL DEFAULT 1,
	local_name character varying,
	display_name character varying,
	description character varying,

	CONSTRAINT pk_categories_events PRIMARY KEY (local_name)
);

insert into ref_categories_events(id, local_name, display_name, description)
values(1, 'tlsgtw_common_ev_type', 'Общие события', 'Общие события');

insert into ref_categories_events(id, local_name, display_name, description)
values(2, 'tlsgtw_authorization_ev_type', 'Авторизация', 'Авторизация в интерфесе управления');

insert into ref_categories_events(id, local_name, display_name, description)
values(4, 'tlsgtw_access_edit_ev_type', 'Редактирование доступа', 'Изменения в списках сертификатов и их ролей, списке доступа пользователей к ресурсам ');

insert into ref_categories_events(id, local_name, display_name, description)
values(8, 'tlsgtw_proxy_res_ev_type', 'Ресурсы', 'Изменения в списке в проксируемых ресурсов');

insert into ref_categories_events(id, local_name, display_name, description)
values(16, 'tlsgtw_settings_ev_type', 'Настройки', 'Изменение настроек TLS Gateway');

insert into ref_categories_events(id, local_name, display_name, description)
values(32, 'tlsgtw_keys_ev_type', 'Ключи', 'Изменение состояния ключа');

insert into ref_categories_events(id, local_name, display_name, description)
values(64, 'tlsgtw_ca_ev_type', 'Сертификаты УЦ', 'Корневые сертификаты УК и CRL');

insert into ref_categories_events(id, local_name, display_name, description)
values(128, 'tlsgtw_backup_ev_type', 'Резервное копирование', 'Резервное копирование');

insert into ref_categories_events(id, local_name, display_name, description)
values(256, 'tlsgtw_system_ev_type', 'Система', 'Системные события TLS Gateway');

insert into ref_categories_events(id, local_name, display_name, description)
values(512, 'tlsgtw_license_ev_type', 'Лицензии', 'События, связаные с лицензиями');

