-- Function: create_task(bigint, character varying, bigint, character varying[])

DROP FUNCTION create_task(bigint, character varying, bigint, character varying[]);

CREATE OR REPLACE FUNCTION create_task(
    task_type_id_in bigint,
    task_type_string_id_in character varying,
    object_id_in bigint,
    args_in character varying[])
  RETURNS table(
	task_id bigint,
	task_type_id bigint,
	task_type_string_id varchar,
	object_id bigint,
	oblect_display_name varchar,
	args varchar[],
	status bigint,
	when_created timestamp with time zone,
	creator_id bigint,
	when_changed_status timestamp with time zone,
	who_changed_status bigint,
	status_args varchar[]
) AS
$BODY$
declare
	subject_id_ bigint;
	task_type_id_ bigint;
	task_id_ bigint;
begin
	-- Получаем ID текущего пользователя
	select into subject_id_ get_current_user_cert_id();

	-- Проверяем корректность параметров функции
	if (task_type_id_in is null and task_type_string_id_in is null) or (object_id_in is null)
	 then
		execute 'select * from add_last_error(-6);';
		return; -- NULL недопустим.
	end if;

	-- Получаем ID типа задания
	if not (task_type_id_in is null) then
		if not exists (select * from ref_type_task where id = task_type_id_in) then -- Если нет такого типа задачи, возвращаем ошибку
			execute 'select * from add_last_error(-8);';
			return; -- Объекта с данным идентификатором нет.
		end if;
		
		task_type_id_ := task_type_id_in;
	else
		select into task_type_id_ RTT.id
		from ref_type_task RTT where RTT.string_id = task_type_string_id_in;
		if not found then -- Если нет такого типа задачи, возвращаем ошибку
			execute 'select * from add_last_error(-2);';
			return; -- Объекта с нужным именем и типом нет.
		end if;
	end if;

	-- Проверяем права пользователя над объектом задания (право на запись)
	if not check_permission_to_object(subject_id_, object_id_in, null, 'w') then
		execute 'select * from add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	begin
		-- Регистрируем задачу как 'Ожидает выполнения'
		insert into system_tasks(task_type_id, object_id, creator_id, args, when_changed_status, who_changed_status)
		values(task_type_id_, object_id_in, subject_id_, args_in, now(), subject_id_)
		returning system_tasks.task_id into task_id_;
		exception when unique_violation then
			execute 'select * from add_last_error(1);'; -- Создаваемый объект уже существует.

		select into task_id_ ST.task_id from system_tasks ST
		where ST.task_type_id = task_type_id_ and ST.object_id = object_id_in and ST.args = args_in;

		if task_id_ is null then
			execute 'select * from add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
			return;
		end if;
	end;

	-- Добавляем событие о создании задачи в лог
	insert into log_system_tasks(task_id, action_type_id, who_done, args)
	values(task_id_, 1, subject_id_, array[subject_id_]);

	return query select
			S.task_id,
			S.task_type_id,
			TT.string_id,
			S.object_id,
			(select get_display_name_by_id(S.object_id)),
			S.args,
			S.status_task,
			cast(S.when_created at time zone 'utc' as timestamptz),
			S.creator_id,
			cast(S.when_changed_status at time zone 'utc' as timestamptz),
			S.who_changed_status,
			(select L.args as args
					from log_system_tasks L
					where L.task_id = S.task_id and L.action_type_id = S.status_task
					order by L.id desc limit 1)
		from system_tasks S
		inner join ref_type_task TT on TT.id = S.task_type_id
		where S.task_id = task_id_;
end;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION create_task(bigint, character varying, bigint, character varying[])
  OWNER TO postgres;
