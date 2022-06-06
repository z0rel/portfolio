-- Function: get_string_name_by_id(bigint)

DROP FUNCTION get_string_name_by_id(bigint);

CREATE OR REPLACE FUNCTION get_string_name_by_id(idobj bigint)
  RETURNS character varying AS
$BODY$
	select case
		when idobj >= 0 and idobj < 4294967296 then
			null -- у сертификата пользователя не может быть уникального строкового идентификатора
		when idobj > 4294967295 and idobj < 8589934592 then
			(select string_id from roles where role_id = idobj)
		when idobj > 8589934591 and idobj < 12884901888 then
			(select external_address from proxy_resources where resource_id = idobj)
		when idobj > 12884901887 and idobj < 17179869184 then
			(select local_name from ref_types_events where event_type_id = idobj)
		when idobj > 17179869183 and idobj < 21474836480 then
			(select local_name from system_objects where obj_id = idobj)
		when idobj > 21474836479 and idobj < 25769803776 then
			null -- у корневого сертификата не может быть уникального строкового идентификатора
		when idobj > 25769803775 and idobj < 30064771072 then
			(select fname from tlsgtw_backups where id = idobj)
		else
			null
		
	end;
$BODY$
  LANGUAGE sql VOLATILE
  COST 100;
ALTER FUNCTION get_string_name_by_id(bigint)
  OWNER TO postgres;
