-- function list_state_reason_for_user()

drop function list_state_reason_for_user();

create or replace function list_state_reason_for_user()
returns table(
	event_category_id bigint,
	event_category_string_id varchar,
	state_id bigint,
	event_type_id bigint,
	event_type_string_id varchar,
	object_id bigint,
	object_display_name varchar,
	msg_id bigint,
	when_reg_first timestamp with time zone,
	when_reg_last timestamp with time zone,
	count bigint,
	args varchar[]) as
$$
declare
	subject_id_ bigint;
begin
	select into subject_id_ get_current_user_cert_id();

	-- Делаем выборку для текущего пользователя
	return query select
		C.id,
		C.local_name,
		E.state,
		E.event_type_id,
		E.local_name,
		U.object_id,
		OBN.display_name,
		U.msg_id,
		cast(U.when_reg_first at time zone 'utc' as timestamptz),
		cast(U.when_reg_last at time zone 'utc' as timestamptz),
		U.count,
		JE.args
	from user_state_reasons U
	inner join ref_types_events E on -- Справочная таблица событий
			E.event_type_id = U.event_type_id and check_permission_to_event(subject_id_, E.event_type_id)
	inner join ref_categories_events C on -- Справочная таблица категорий событий
			C.local_name = E.local_name_category
	inner join journal_event JE on JE.id = U.msg_id
	inner join lateral(
                    select UC.cert_id as object_id, UC.subject_cn as display_name from users_certificates UC where UC.cert_id = U.object_id
               union distinct
                    select R.role_id as object_id, R.display_name as display_name from roles R where R.role_id = U.object_id
               union distinct
                    select PR.resource_id as object_id, PR.resource_name as display_name from proxy_resources PR where PR.resource_id = U.object_id
               union distinct
                    select TRC.root_cert_id as object_id, TRC.subject_cn as display_name from trusted_root_certificate TRC where TRC.root_cert_id = U.object_id
                    ) OBN on OBN.object_id = U.object_id
	where check_permission_to_object(subject_id_, U.object_id, null, 'r');
end;
$$
language plpgsql;