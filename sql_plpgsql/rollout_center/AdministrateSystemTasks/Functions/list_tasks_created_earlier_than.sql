-- Function: list_tasks_created_earlier_than(bigint, bigint, bigint, bigint, bigint, character varying[], bigint, bigint)

DROP FUNCTION list_tasks_created_earlier_than(bigint, bigint, bigint, bigint, bigint, character varying[], bigint, bigint);

CREATE OR REPLACE FUNCTION list_tasks_created_earlier_than(
    IN upper_task_id_in bigint default null,
    IN max_count_in bigint default null,
    IN lower_task_id_in bigint default null,
    IN state_id_mask_in bigint default null,
    IN task_type_id_in bigint default null,
    IN args_in character varying[] default null,
    IN creator_id_in bigint default null,
    IN object_id_in bigint default null)
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
	
	query_filter := '';
	if not (upper_task_id_in is null) then
		query_filter := query_filter || ' task_id < ' || upper_task_id_in;
	end if;

	if not (lower_task_id_in is null) then
		if (char_length(query_filter) > 0) then
			query_filter := query_filter || ' and';
		end if; 
		query_filter := query_filter || ' task_id >= ' || lower_task_id_in;
	end if;

	if not (state_id_mask_in is null) then
		if (char_length(query_filter) > 0) then
			query_filter := query_filter || ' and';
		end if;
		query_filter := query_filter || ' status_task & ' || state_id_mask_in || ' > 0';
	end if;

	if not (task_type_id_in is null) then
		if (char_length(query_filter) > 0) then
			query_filter := query_filter || ' and';
		end if;
		query_filter := query_filter || ' task_type_id & ' || task_type_id_in || ' > 0';
	end if;

	if not (args_in is null) then
		if (char_length(query_filter) > 0) then
			query_filter := query_filter || ' and';
		end if;
		query_filter := query_filter || ' args = ' || args_in;
	end if;

	if not (creator_id_in is null) then
		if (char_length(query_filter) > 0) then
			query_filter := query_filter || ' and';
		end if;
		query_filter := query_filter || ' creator_id = ' || creator_id_in;
	end if;

	if not (object_id_in is null) then
		if (char_length(query_filter) > 0) then
			query_filter := query_filter || ' and';
		end if;
		query_filter := query_filter || ' object_id = ' || object_id_in;
	end if;

	if not (char_length(query_filter) > 0) then
		return query select
			S.task_id,
			S.task_type_id,
			TT.string_id,
			S.object_id,
			(select get_display_name_by_id(S.object_id)),
			S.args,
			S.status_task,
			cast(S.when_created at time zone 'utc' as timestamptz),
			S.creator_id,
			cast(S.when_changed_status at time zone 'utc' as timestamptz),
			S.who_changed_status,
			(select UC.serial_number from users_certificates UC where UC.cert_id = S.who_changed_status),
			(select UC.issuer_dn from users_certificates UC where UC.cert_id = S.who_changed_status),
			(select L.args as args
					from log_system_tasks L
					where L.task_id = S.task_id and L.action_type_id = S.status_task
					order by L.id desc limit 1)
		from system_tasks S
		inner join ref_type_task TT on TT.id = S.task_type_id
		where check_permission_to_object(subject_id_, S.object_id, null, 'r')
		order by S.task_id desc limit max_count_in;
	else
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
		order by S.task_id desc limit ' || quote_nullable(max_count_in) || ';';

		return query execute query_text_;
	end if;
end;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100
  ROWS 1000;
ALTER FUNCTION list_tasks_created_earlier_than(bigint, bigint, bigint, bigint, bigint, character varying[], bigint, bigint)
  OWNER TO postgres;
