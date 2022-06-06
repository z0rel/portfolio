-- Table: log_system_tasks

DROP TABLE log_system_tasks;

CREATE TABLE log_system_tasks
(
	id bigserial NOT NULL,
	task_id bigint,
	action_type_id bigint NOT NULL,
	when_done timestamp with time zone NOT NULL DEFAULT now(),
	who_done bigint,
	args character varying[],
	CONSTRAINT log_system_tasks_id_pk PRIMARY KEY (id)
)
WITH (
	OIDS=FALSE
);
ALTER TABLE log_system_tasks
	OWNER TO postgres;
