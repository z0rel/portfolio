-- Function: list_events_earlier_than(bigint, bigint, bigint, bigint, bigint, bigint, bigint)

DROP FUNCTION list_events_earlier_than(bigint, bigint, bigint, bigint, bigint, bigint, bigint);

CREATE OR REPLACE FUNCTION list_events_earlier_than(
    IN upper_msg_id_in bigint,
    IN max_count_in bigint,
    IN lower_msg_id_in bigint,
    IN severity_id_mask_in bigint,
    IN event_cathegory_id_mask_in bigint,
    IN subject_id_in bigint,
    IN object_id_in bigint)
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
$BODY$
declare
	query_filter text;
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

	if not (subject_id_in is null) then
		if (char_length(query_filter) > 0) then
			query_filter := query_filter || ' and';
		end if;
		query_filter := query_filter || ' J.subject_id = ' || subject_id_in;
	end if;

	if not (object_id_in is null) then
		if (char_length(query_filter) > 0) then
			query_filter := query_filter || ' and';
		end if;
		query_filter := query_filter || ' J.object_id = ' || object_id_in;
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
				get_display_name_by_id(J.subject_id) as subject_display_name,
			    get_type_object_by_id(J.subject_id, J.subject_role_group) as subject_type,
			    RS.roles as subject_roles,
				J.object_id as object_id,
				get_display_name_by_id(J.object_id, J.object_role_group) as object_display_name,
			    get_type_object_by_id(J.object_id) as object_type,
			    RO.roles as object_roles,
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
					left join lateral (
										select array_agg(R.string_id) as roles
										from roles R
										where R.role_id in (
											select URE.role_id
											from user_roles_entries URE
											where URE.user_cert_id = J.subject_id
										)
										) as RS on true
					left join lateral (
										select array_agg(R.string_id) as roles
										from roles R
										where R.role_id in (
											select URE.role_id
											from user_roles_entries URE
											where URE.user_cert_id = J.object_id
										)
										) as RO on true
				order by J.id desc limit max_count_in;
	else
		return query execute 'select J.id as msg_id,
				cast(J.reg_time at time zone ''utc'' as timestamptz) as when_registered,
				J.event_type_id as event_type_id,
				E.local_name as event_type_string_id,
				C.id as category_id,
				C.local_name as category_string_id,
				E.state as event_severity_id,
				J.subject_id as subject_id,
				get_display_name_by_id(J.subject_id) as subject_display_name,
			    get_type_object_by_id(J.subject_id, J.subject_role_group) as subject_type,
			    RS.roles as subject_roles,
				J.object_id as object_id,
				get_display_name_by_id(J.object_id) as object_display_name,
			    get_type_object_by_id(J.object_id, J.object_role_group) as object_type,
			    RO.roles as object_roles,
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
					left join lateral (
										select array_agg(R.string_id) as roles
										from roles R
										where R.role_id in (
											select URE.role_id
											from user_roles_entries URE
											where URE.user_cert_id = J.subject_id
										)
										) as RS on true
					left join lateral (
										select array_agg(R.string_id) as roles
										from roles R
										where R.role_id in (
											select URE.role_id
											from user_roles_entries URE
											where URE.user_cert_id = J.object_id
										)
										) as RO on true

				where ' || query_filter || '
				order by J.id desc limit ' || quote_nullable(max_count_in) || ';';
	end if;
end;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100
  ROWS 1000;
ALTER FUNCTION list_events_earlier_than(bigint, bigint, bigint, bigint, bigint, bigint, bigint)
  OWNER TO postgres;
