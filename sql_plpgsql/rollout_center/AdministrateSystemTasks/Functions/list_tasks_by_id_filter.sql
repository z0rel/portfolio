-- Function: list_tasks_by_id_filter(bigint[])

DROP FUNCTION list_tasks_by_id_filter(bigint[]);

CREATE OR REPLACE FUNCTION list_tasks_by_id_filter(tasks_ids_in bigint[] default null)
  RETURNS TABLE(task_id bigint,
			task_type_id bigint,
			task_type_string_id character varying,
			object_id bigint,
			oblect_display_name character varying,
			args character varying[],
			status bigint,
			when_created timestamp with time zone,
			creator_id bigint,
			when_changed_status timestamp with time zone,
			who_changed_status bigint,
			initiator_certificate_serial varchar,
			initiator_certificate_issuer varchar,
			status_args varchar[]) AS
$BODY$
declare
	subject_id_ bigint;
	query_text_ text;
	query_filter text;
begin
	-- Получаем ID текущего пользователя
	select into subject_id_ get_current_user_cert_id();

	if not (tasks_ids_in is null) then
		query_filter := 'S.task_id = any(ARRAY[' || array_to_string(tasks_ids_in, ',', 'null') || ']::bigint[])';
	else
		query_filter := 'true';
	end if;

	query_text_ := 'select
		S.task_id,
		S.task_type_id,
		TT.string_id,
		S.object_id,
		(select get_display_name_by_id(S.object_id)),
		S.args,
		S.status_task,
		cast(S.when_created at time zone ''utc'' as timestamptz),
		S.creator_id,
		cast(S.when_changed_status at time zone ''utc'' as timestamptz),
		S.who_changed_status,
		(select UC.serial_number from users_certificates UC where UC.cert_id = S.who_changed_status),
		(select UC.issuer_dn from users_certificates UC where UC.cert_id = S.who_changed_status),
		(select L.args as args
				from log_system_tasks L
				where L.task_id = S.task_id and L.action_type_id = S.status_task
				order by L.id desc limit 1)
	from system_tasks S
	inner join ref_type_task TT on TT.id = S.task_type_id
	where ' || query_filter || ' and check_permission_to_object(' || quote_nullable(subject_id_) || ', S.object_id, null, ''r'')
	order by S.task_id desc limit 1000000;';

	return query execute query_text_;

end;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100
  ROWS 1000;
ALTER FUNCTION list_tasks_by_id_filter(bigint[])
  OWNER TO postgres;
