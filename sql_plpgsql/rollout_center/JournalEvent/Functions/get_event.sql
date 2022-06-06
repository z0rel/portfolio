-- Function: get_event(bigint)

DROP FUNCTION get_event(bigint);

CREATE OR REPLACE FUNCTION get_event(IN msg_id_in bigint)
  RETURNS TABLE(
    msg_id bigint,
		when_registered timestamp with time zone,
    event_type_id bigint,
    event_type_string_id character varying,
    category_id bigint,
    category_string_id character varying,
    event_severity_id bigint,

    subject_id bigint,
    subject_display_name character varying,
    object_id bigint,
    object_display_name character varying,
    args character varying[],
    result boolean,
    is_processed boolean) AS
$BODY$
declare
	subject_id_ bigint;
	subject_display_name varchar;
	object_id_ bigint;
	object_display_name varchar;
	is_processed boolean;
begin
	-- Если события с данным ID нет, то возвращаем пустой результат
	if not exists (select * from journal_event where id = msg_id_in) then
		return;
	end if;

	-- Получаем subject_id
	select into subject_id_ journal_event.subject_id from journal_event where id = msg_id_in;

	-- Получаем subject_display_name
	select into subject_display_name get_display_name_by_id(subject_id_);

	-- Получаем object_id
	select into object_id_ journal_event.object_id from journal_event where id = msg_id_in;

	-- Получаем object_display_name
	select into object_display_name get_display_name_by_id(object_id_);

	-- Получаем is_processed
	select into is_processed exists (
		select *
		from current_msg_id_entry
		where current_msg_id_entry.msg_id > msg_id_in or current_msg_id_entry.msg_id = msg_id_in);

	-- Возвращаем результат
	return query select
		J.id,
		cast(J.reg_time at time zone 'utc' as timestamptz),
		J.event_type_id,
		E.local_name,
		C.id,
		C.local_name,
		E.state,
		J.subject_id,
		subject_display_name,
		J.object_id,
		object_display_name,
		J.args,
		J.result,
		is_processed
	from journal_event J
	inner join ref_types_events E on E.event_type_id = J.event_type_id
	inner join ref_categories_events C on C.local_name = E.local_name_category
	where J.id = msg_id_in;
end;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100
  ROWS 1000;
ALTER FUNCTION get_event(bigint)
  OWNER TO postgres;
