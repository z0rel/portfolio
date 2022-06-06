-- Table: ref_type_task

DROP TABLE ref_type_task;

CREATE TABLE ref_type_task
(
	id bigint NOT NULL,
	string_id varchar,
	display_name varchar,
	args varchar[],
	CONSTRAINT pk_type_task_id PRIMARY KEY (id)
);

insert into ref_type_task(id, string_id, display_name, args)
values(1,'create_backup','Создание резервной копии', '{"Имя файла резервной копии"}');

insert into ref_type_task(id, string_id, display_name, args)
values(2,'reload_nginx_config','Проверка новых параметров nginx, их применение и перезапуск nginx', '{}');

insert into ref_type_task(id, string_id, display_name, args)
values(4,'import_license','Импорт лицензии', '{"Лицензия"}');

insert into ref_type_task(id, string_id, display_name, args)
values(8,'install_sw_update','Установка обновления ПО', '{"Имя файла обновления"}');


