-- function add_last_error( bigint );

drop function add_last_error( bigint );

create or replace function add_last_error(err bigint)
returns void as
$$
begin
	-- Проверяем наличие временной таблицы tt_last_error
	if not exists (select relname from pg_class where relname = 'tt_last_error') then
		return;
	end if;

	delete from tt_last_error;
	insert into tt_last_error(error_id, error_str) values (err, (select err_str from ref_code_errors where err_code = err));
	return;
end;
$$
language plpgsql;