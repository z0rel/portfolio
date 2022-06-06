-- Table: system_tasks

DROP TABLE system_tasks;

CREATE TABLE system_tasks
(
	task_id bigserial NOT NULL,
	task_type_id bigint NOT NULL,
	status_task bigint NOT NULL DEFAULT 1,
	object_id bigint NOT NULL,
	creator_id bigint NOT NULL,
	when_created timestamp with time zone NOT NULL DEFAULT now(),
	when_changed_status timestamp with time zone,
	who_changed_status bigint,
	args character varying[],
	CONSTRAINT system_tasks_task_id_pk PRIMARY KEY (task_id),
	constraint system_tasks_type_obj_args_uq unique(task_type_id, object_id, args)
)
WITH (
	OIDS=FALSE
);
ALTER TABLE system_tasks
	OWNER TO postgres;