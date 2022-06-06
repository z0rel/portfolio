-- function array_intersect(bigint[], bigint[])

drop function array_intersect(bigint[], bigint[]);

create or replace function array_intersect(arr1 bigint[], arr2 bigint[])
returns bigint[] as
$$
begin
	return array(select unnest(arr1) intersect select unnest(arr2));
end;
$$
language plpgsql;