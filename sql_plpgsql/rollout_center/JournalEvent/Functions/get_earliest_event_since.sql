-- Function: get_earliest_event_since(timestamp with time zone)

DROP FUNCTION get_earliest_event_since(timestamp with time zone);

CREATE OR REPLACE FUNCTION get_earliest_event_since(since_date_in timestamp with time zone)
  RETURNS bigint AS
$BODY$
	select min(id)
	from journal_event
	where reg_time >= since_date_in;
$BODY$
  LANGUAGE sql VOLATILE
  COST 100;
ALTER FUNCTION get_earliest_event_since(timestamp with time zone)
  OWNER TO postgres;
COMMENT ON FUNCTION get_earliest_event_since(timestamp with time zone) IS 'Выдает идентификатор самого раннего события, которое случилось начиная с некоей даты включительно. Событие вычисляется безотносительно прав доступа на объект события.';
