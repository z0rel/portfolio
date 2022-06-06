-- Function: list_all_events()

DROP FUNCTION list_all_events();

CREATE OR REPLACE FUNCTION list_all_events()
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
	msg_id_ bigint;
begin
	for msg_id_ in (select JE.id from journal_event JE) loop
		return query select * from get_event(msg_id_) limit 1000000;
	end loop;
end;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100
  ROWS 1000;
ALTER FUNCTION list_all_events()
  OWNER TO postgres;
