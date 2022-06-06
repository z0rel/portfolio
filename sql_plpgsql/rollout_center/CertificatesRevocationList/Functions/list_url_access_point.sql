-- function list_url_access_point()

drop function list_url_access_point();

create or replace function list_url_access_point()
	returns table (url varchar) as
$$
begin
	return query select TRC.crl_access_point from trusted_root_certificate TRC where TRC.is_active;
end;
$$
language plpgsql;