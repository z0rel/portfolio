-- function get_cert_type_by_role_group(varchar, varchar);

drop function get_cert_type_by_role_group(role_group);

create or replace function get_cert_type_by_role_group(cert_role_group_in role_group)
returns character varying as
$$
begin
	if (cert_role_group_in = 'admin') then
			return 'admin_certificate';
	elseif (cert_role_group_in = 'user') then
			return 'user_certificate';
	else
			return 'certificate';
	end if;
end;
$$
language plpgsql;