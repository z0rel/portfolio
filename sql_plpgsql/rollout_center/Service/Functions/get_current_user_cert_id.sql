-- function get_current_user_cert_id();

drop function get_current_user_cert_id();

create or replace function get_current_user_cert_id()
returns bigint as
$$
declare
	subject_id bigint;
begin
	-- Получаем ID субъекта из временной таблицы tt_subject_id
	if exists (select relname from pg_class where relname = 'tt_subject_id') then
		select into subject_id id from tt_subject_id;
	else
		select into subject_id null;
	end if;
	
	return subject_id;
end;
$$
language plpgsql;