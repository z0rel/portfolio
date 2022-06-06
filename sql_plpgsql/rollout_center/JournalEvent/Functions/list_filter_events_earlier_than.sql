-- DROP FUNCTION list_filter_events_earlier_than(bigint,bigint,bigint,bigint,bigint,timestamp with time zone,timestamp with time zone,bigint[],varchar[],bigint[],varchar[],boolean);

DROP FUNCTION list_filter_events_earlier_than(bigint,bigint,bigint,bigint,bigint,timestamp with time zone,timestamp with time zone,bigint[],varchar[], bigint[],varchar[],boolean);

CREATE OR REPLACE FUNCTION list_filter_events_earlier_than(
    upper_msg_id_in bigint,
    max_count_in bigint,
    lower_msg_id_in bigint,
    severity_id_mask_in bigint,
    event_cathegory_id_mask_in bigint,
    event_from_in timestamptz,
    event_to_in timestamptz,
    subjects_in bigint[],
		subject_types varchar[],
    objects_in bigint[],
    events_in varchar[],
		result_in boolean
)
  RETURNS TABLE(msg_id bigint,
	when_registered timestamp with time zone,
	event_type_id bigint,
	event_type_string_id character varying,
	category_id bigint,
	category_string_id character varying,
	event_severity_id bigint,
	subject_id bigint,
	subject_display_name character varying,
	subject_type character varying,
	subject_roles character varying[],
	object_id bigint,
	object_display_name character varying,
	object_type character varying,
	object_roles character varying[],
	args character varying[],
  result boolean,
	is_processed boolean) AS
$$
declare
	query_filter text;
  	query_text text;
	str_arr_low varchar[];
	subject_id_ bigint;
begin
  -- Получаем ID текущего пользователя
	select into subject_id_ get_current_user_cert_id();

	query_filter := '';
	if not (upper_msg_id_in is null) then
		query_filter := query_filter || ' J.id < ' || upper_msg_id_in;
	end if;

	if not (lower_msg_id_in is null) then
		if (char_length(query_filter) > 0) then
			query_filter := query_filter || ' and';
		end if;
		query_filter := query_filter || ' J.id >= ' || lower_msg_id_in;
	end if;

	if not (severity_id_mask_in is null) and severity_id_mask_in > 0 then
		if (char_length(query_filter) > 0) then
			query_filter := query_filter || ' and';
		end if;
		query_filter := query_filter || ' E.state & ' || severity_id_mask_in || ' > 0';
	end if;

	if not (event_cathegory_id_mask_in is null) and event_cathegory_id_mask_in > 0 then
		if (char_length(query_filter) > 0) then
			query_filter := query_filter || ' and';
		end if;
		query_filter := query_filter || ' C.id & ' || event_cathegory_id_mask_in || ' > 0';
	end if;

	if not (event_from_in is null) then
		if (char_length(query_filter) > 0) then
			query_filter := query_filter || ' and';
		end if;
		query_filter := query_filter || ' J.reg_time >= ''' || event_from_in || '''';
	end if;

	if not (event_to_in is null) then
		if (char_length(query_filter) > 0) then
			query_filter := query_filter || ' and';
		end if;
		query_filter := query_filter || ' J.reg_time <= ''' || event_to_in || '''';
	end if;

  if not (subjects_in is null) and ( array_length(subjects_in, 1) >= 1 ) then
		if (char_length(query_filter) > 0) then
			query_filter := query_filter || ' and';
		end if;
		query_filter := query_filter || ' J.subject_id = any(ARRAY[' || array_to_string(subjects_in, ',', 'null') || ']::bigint[])';
	end if;

  if not (subject_types is null) and ( array_length(subject_types, 1) >= 1 ) then
		if (char_length(query_filter) > 0) then
			query_filter := query_filter || ' and';
		end if;
    select into str_arr_low lower_array_str(subject_types);
		query_filter := query_filter || ' get_type_object_by_id(J.subject_id, null) = any(ARRAY[''' || array_to_string(str_arr_low, ''',''', 'null') || ''']::varchar[])';
	end if;

  if not (objects_in is null) and ( array_length(objects_in, 1) >= 1 ) then
		if (char_length(query_filter) > 0) then
			query_filter := query_filter || ' and';
		end if;
		query_filter := query_filter || ' J.object_id = any(ARRAY[' || array_to_string(objects_in, ',', 'null') || ']::bigint[])';
	end if;

  if not (events_in is null) and ( array_length(events_in, 1) >= 1 ) then
		if (char_length(query_filter) > 0) then
			query_filter := query_filter || ' and';
		end if;
    select into str_arr_low lower_array_str(events_in);
		query_filter := query_filter || ' lower(E.local_name) = any(ARRAY[''' || array_to_string(str_arr_low, ''',''', 'null') || ''']::varchar[])';
	end if;

  if not (result_in is null) then
		if (char_length(query_filter) > 0) then
			query_filter := query_filter || ' and';
		end if;
		query_filter := query_filter || ' J.result = ' || result_in || '';
	end if;

  if char_length(query_filter) = 0 then
		return query select J.id as msg_id,
				cast(J.reg_time at time zone 'utc' as timestamptz) as when_registered,
				J.event_type_id as event_type_id,
				E.local_name as event_type_string_id,
				C.id as category_id,
				C.local_name as category_string_id,
				E.state as event_severity_id,
				J.subject_id as subject_id,
				(select get_display_name_by_id(J.subject_id)) as subject_display_name,
				(select get_type_object_by_id(J.subject_id, J.subject_role_group)) as subject_type,
				(select array_agg(R.string_id)
					from roles R
					where R.role_id in (
									select URE.role_id
									from user_roles_entries URE
									where URE.user_cert_id = J.subject_id
					)) as subject_roles,
				J.object_id as object_id,
				(select get_display_name_by_id(J.object_id)) as object_display_name,
        (select get_type_object_by_id(J.object_id, J.object_role_group)) as object_type,
				(select array_agg(R.string_id) as roles
					from roles R
					where R.role_id in (
									select URE.role_id
									from user_roles_entries URE
									where URE.user_cert_id = J.object_id
					)) as object_roles,
				J.args as args,
        J.result,
				RP.V as is_processed
				from journal_event J
				inner join lateral(
					select cast(
						exists(
							select CMID.msg_id from current_msg_id_entry CMID where CMID.msg_id >= J.id
							) as boolean
						) as V
					) RP on RP.V = true
				inner join ref_types_events E on E.event_type_id = J.event_type_id and check_permission_to_event(subject_id_, E.event_type_id)
				inner join ref_categories_events C on C.local_name = E.local_name_category
				order by J.id desc limit max_count_in;
  else
    query_text := 'select J.id as msg_id,
				cast(J.reg_time at time zone ''utc'' as timestamptz) as when_registered,
				J.event_type_id as event_type_id,
				E.local_name as event_type_string_id,
				C.id as category_id,
				C.local_name as category_string_id,
				E.state as event_severity_id,
				J.subject_id as subject_id,
				(select get_display_name_by_id(J.subject_id)) as subject_display_name,
				(select get_type_object_by_id(J.subject_id, J.subject_role_group)) as subject_type,
				(select array_agg(R.string_id)
					from roles R
					where R.role_id in (
									select URE.role_id
									from user_roles_entries URE
									where URE.user_cert_id = J.subject_id
					)) as subject_roles,
				J.object_id as object_id,
				coalesce((select get_display_name_by_id(J.object_id)), J.args[1]) as object_display_name,
                    (select get_type_object_by_id(J.object_id, J.object_role_group)) as object_type,
				(select array_agg(R.string_id) as roles
					from roles R
					where R.role_id in (
									select URE.role_id
									from user_roles_entries URE
									where URE.user_cert_id = J.object_id
					)) as object_roles,
				J.args as args,
				J.result,
				RP.V as is_processed
				from journal_event J
				inner join lateral(
					select cast(
						exists(
							select CMID.msg_id from current_msg_id_entry CMID where CMID.msg_id >= J.id
							) as boolean
						) as V
					) RP on RP.V = true
				inner join ref_types_events E on E.event_type_id = J.event_type_id and check_permission_to_event(' || quote_nullable(subject_id_) || ', E.event_type_id)
				inner join ref_categories_events C on C.local_name = E.local_name_category
				where ' || query_filter || '
				order by J.id desc limit ' || quote_nullable(max_count_in) || ';';

    return query execute query_text;
  end if;

end
$$ language plpgsql;