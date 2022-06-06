-- function list_crl_dependent_services()

drop function list_crl_dependent_services();

create or replace function list_crl_dependent_services()
	returns table (service_name varchar, desired_state bigint) as
$$
begin
	return query select DS.service_name, DS.state from crl_dependent_services DS;
end;
$$
language plpgsql;