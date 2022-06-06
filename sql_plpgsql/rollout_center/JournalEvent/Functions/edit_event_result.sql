drop function edit_event_result(bigint, boolean);

create or replace function edit_event_result(event_id_in bigint, result_in boolean)
returns boolean as
$$
declare
begin
  if not exists (select * from journal_event  JE where JE.id = event_id_in) then
		execute 'select add_last_error(-8);'; --Объекта с данным идентификатором нет
		return false;
	end if;

  update journal_event JE set JE.result = coalesce(result_in, JE.result)
	where JE.id = event_id_in;

	return true;
end;
$$
language plpgsql;