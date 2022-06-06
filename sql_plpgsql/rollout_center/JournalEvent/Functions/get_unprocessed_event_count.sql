-- Function: get_unprocessed_event_count()

DROP FUNCTION get_unprocessed_event_count();

CREATE OR REPLACE FUNCTION get_unprocessed_event_count()
  RETURNS bigint AS
$BODY$
declare
	cur_msg_id bigint;
	last_msg_id bigint;
begin
	select into cur_msg_id msg_id from current_msg_id_entry;
	select into last_msg_id id from journal_event order by id desc limit 1;

	if last_msg_id > cur_msg_id then
		return last_msg_id - cur_msg_id;
	end if;

	return 0;
end;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION get_unprocessed_event_count()
  OWNER TO postgres;
