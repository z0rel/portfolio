-- Function: get_display_name_by_id(bigint)

DROP FUNCTION get_display_name_by_id(bigint);

create or replace function get_display_name_by_id(idobj bigint)
returns varchar as
$$
	select case
		when idobj >= 0 and idobj < 4294967296 then
			(select subject_cn from users_certificates where cert_id = idobj)
		when idobj > 4294967295 and idobj < 8589934592 then
			(select display_name from roles where role_id = idobj)
		when idobj > 8589934591 and idobj < 12884901888 then
			(select resource_name from proxy_resources where resource_id = idobj)
		when idobj > 12884901887 and idobj < 17179869184 then
			(select description from ref_types_events where event_type_id = idobj)
		when idobj > 17179869183 and idobj < 21474836480 then
			(select display_name from system_objects where obj_id = idobj)
		when idobj > 21474836479 and idobj < 25769803776 then
			(select subject_cn from trusted_root_certificate where root_cert_id = idobj)
		when idobj > 25769803775 and idobj < 30064771072 then
			(select fname from tlsgtw_backups where id = idobj)
		else
			null

	end;

$$
language sql;