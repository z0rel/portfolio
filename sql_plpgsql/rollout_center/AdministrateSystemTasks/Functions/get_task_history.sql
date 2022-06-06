-- function get_task_history(bigint);

DROP FUNCTION get_task_history(bigint);

create or replace function get_task_history ( task_id_in bigint  )
returns table(
id bigint,
task_id bigint,
action_type_id bigint,
action_type_string_id varchar,
args varchar[],
when_done timestamp with time zone,
who_done bigint,
who_done_display_name varchar
) as
$$
declare
	subject_id_ bigint;
	object_id_ bigint;
begin
	-- Получаем ID текущего пользователя
	select into subject_id_ get_current_user_cert_id();

	-- Проверяем существование задачи с данным идентификатором
	if not exists (select * from log_system_tasks L where L.task_id = task_id_in) then
		execute 'select * from add_last_error(-8);'; -- Объекта с данным идентификатором нет.
		return;
	end if;

	-- Получаем ID объекта задания
	select into object_id_ object_id from system_tasks S where S.task_id = task_id_in;

	-- Проверяем права пользователя на объект задачи
	if not check_permission_to_object(subject_id_, object_id_, 'r') then
		execute 'select * from add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	return query select
			L.id,
			L.task_id,
			L.action_type_id,
			SATT.string_id,
			L.args,
			cast(L.when_done at time zone 'utc' as timestamptz),
			L.who_done,
			(select get_display_name_by_id(L.who_done))
		from log_system_tasks L
		inner join ref_state_action_types_tasks SATT on SATT.id = L.action_type_id
		where L.task_id = task_id_in;
end;
$$
language plpgsql;