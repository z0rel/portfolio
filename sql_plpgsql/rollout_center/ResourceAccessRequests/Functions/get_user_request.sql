-- Function: get_user_request()

DROP FUNCTION get_user_request();

CREATE OR REPLACE FUNCTION get_user_request(
	OUT request_description_out varchar,
	OUT user_email_out varchar,
	OUT resources_out bigint[]) AS
$BODY$
declare
	subject_id_ bigint;
	last_modify_date_ timestamptz;
begin
	-- Получаем ID текущего сертификата пользователя
	select into subject_id_ get_current_user_cert_id();

	select
		RAR.request_description,
		RAR.user_email,
		array(select RRE.requested_resource_id from resources_requests_entries RRE where RRE.user_cert_id = subject_id_)
	from resources_access_requests RAR
	where RAR.user_cert_id = subject_id_
	into request_description_out, user_email_out, resources_out;

end;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION get_user_request()
  OWNER TO postgres;
COMMENT ON FUNCTION get_user_request() IS 'Получение запроса на ресурсы текущего пользователя.';
