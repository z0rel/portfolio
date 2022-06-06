-- Table: ref_state_action_types_tasks

DROP TABLE ref_state_action_types_tasks;

CREATE TABLE ref_state_action_types_tasks
(
	id bigint NOT NULL,
	string_id character varying,
	display_name character varying,
	description character varying,
	args character varying[]
)
WITH (
	OIDS=FALSE
);
ALTER TABLE ref_state_action_types_tasks
	OWNER TO postgres;

insert into ref_state_action_types_tasks(id, string_id, display_name, description, args)
values(1, 'created', 'Ожидает выполнения', 'Задача была создана, но еще ни разу не выполнялась.', '{"ID учетной записи, иницировавшей задачу"}');

insert into ref_state_action_types_tasks(id, string_id, display_name, description, args)
values(2, 'working', 'Выполняется', 'Задача выполняется в текущий момент.', '{}');

insert into ref_state_action_types_tasks(id, string_id, display_name, description, args)
values(4, 'will_be_repeated', 'Ожидает повтора', 'Задача выполнялась как минимум 1 раз, выполнение не удалось по временным причинам, допускающим успешное выполнение при автоматическом повторе.', '{"Описание ошибки"}');

insert into ref_state_action_types_tasks(id, string_id, display_name, description, args)
values(8, 'completed_with_error', 'Выполнена с ошибкой', 'Выполнение задачи оказалось невозможным по причинам, которые невозможно обойти повторным выполнением. Для устранения причин требуются действия оператора.', '{"Описание ошибки"}');

insert into ref_state_action_types_tasks(id, string_id, display_name, description, args)
values(16, 'completed_successfully', 'Выполнена успешно', 'Задача завершилась успешно (не обязательно с первой попытки).', '{}');

insert into ref_state_action_types_tasks(id, string_id, display_name, description, args)
values(32, 'canceled', 'Отменена', 'Задача была отменена по команде пользователя (не обязательно того же, кто создал задачу).', '{"ID учетной записи, отменившей задачу"}');

insert into ref_state_action_types_tasks(id, string_id, display_name, description, args)
values(64, 'recreated', 'Создана повторно', 'Существующая задача была создана повторно, либо возобновлена после остановки.', '{"ID учетной записи, повторно запустившей задачу"}');

insert into ref_state_action_types_tasks(id, string_id, display_name, description, args)
values(128, 'cancelling', 'Отменяется', 'Поступила команда на остановку задачи, но задача еще не остановилась.', '{"ID учетной записи, отменившей задачу"}');
