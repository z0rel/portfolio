-- Function: get_latest_event_upto(timestamp with time zone)

DROP FUNCTION get_latest_event_upto(timestamp with time zone);

CREATE OR REPLACE FUNCTION get_latest_event_upto(upto_date_in timestamp with time zone)
  RETURNS bigint AS
$BODY$
	select max(id)
	from journal_event
	where reg_time <= upto_date_in;
$BODY$
  LANGUAGE sql VOLATILE
  COST 100;
ALTER FUNCTION get_latest_event_upto(timestamp with time zone)
  OWNER TO postgres;
COMMENT ON FUNCTION get_latest_event_upto(timestamp with time zone) IS 'Выдает идентификатор самого позднего события, по указанную дату. Событие вычисляется безотносительно прав доступа на объект события.';
