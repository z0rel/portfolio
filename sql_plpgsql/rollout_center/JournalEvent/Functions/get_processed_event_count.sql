-- Function: get_processed_event_count(bigint, bigint, bigint, bigint, bigint, bigint)

-- DROP FUNCTION get_processed_event_count(bigint, bigint, bigint, bigint, bigint, bigint);

CREATE OR REPLACE FUNCTION get_processed_event_count(
    upper_msg_id_in bigint,
    lower_msg_id_in bigint,
    severity_id_mask_in bigint,
    event_cathegory_id_mask_in bigint,
    subject_id_in bigint,
    object_id_in bigint)
  RETURNS bigint AS
$BODY$
	select count(*) from list_events_earlier_than($1, null, $2, $3, $4, $5, $6);
$BODY$
  LANGUAGE sql VOLATILE
  COST 100;
ALTER FUNCTION get_processed_event_count(bigint, bigint, bigint, bigint, bigint, bigint)
  OWNER TO postgres;
