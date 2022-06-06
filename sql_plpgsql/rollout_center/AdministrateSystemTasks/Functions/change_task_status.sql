DROP FUNCTION change_task_status(bigint,bigint,character varying[]);

create or replace function change_task_status ( task_id_in bigint, new_status_in bigint, status_args_in varchar[] )
returns table(
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
initiator_certificate_serial varchar,
initiator_certificate_issuer varchar,
status_args varchar[]
) as
$$
declare
	subject_id_ bigint;
	args_ varchar[];
begin
	-- Получаем ID текущего пользователя
	select into subject_id_ get_current_user_cert_id();
	
	-- Проверяем наличие задачи
	if not exists (select S.task_id from system_tasks S where S.task_id = task_id_in) then
		execute 'select * from add_last_error(-8);'; -- Объекта с данным идентификатором нет.
		return;
	end if;

	-- Статус 'created' может быть установлен только автоматически
	if new_status_in = 1 then
		execute 'select * from add_last_error(-28);'; -- Данное свойство устанавливается только автоматически.
		return;
	end if;

	-- Проверяем право пользователя на объект задачи
	if not check_permission_to_object(subject_id_, (select S.object_id from system_tasks S where S.task_id = task_id_in), null, 'r') then
		execute 'select * from add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;	
	end if;

	-- Добавляем для статусов, требующих ID пользователя текущего, соответствующий аргумент
	if new_status_in = 1 or new_status_in = 32 or new_status_in = 64 or new_status_in = 128 then
		args_ := array[subject_id_];
	else
		args_ := status_args_in;
	end if;

	-- Проверяем валидность статуса
	if not new_status_in is null then
		if not exists (select * from ref_state_action_types_tasks where id = new_status_in) then
			execute 'select * from add_last_error(-27);'; -- Свойства с таким именем или идентификатором нет.
			return;
		end if;

		update system_tasks S
		set status_task = new_status_in,
				when_changed_status = now(),
				who_changed_status = subject_id_
		where S.task_id = task_id_in;

		if new_status_in in (select id from ref_state_action_types_tasks where string_id = 'recreated') then
			update system_tasks S
				set creator_id = subject_id_
			where S.task_id = task_id_in;
		end if;

		insert into log_system_tasks(task_id, action_type_id, who_done, args)
		values(task_id_in, new_status_in, subject_id_, args_);
	end if;

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
			(select UC.serial_number from users_certificates UC where UC.cert_id = S.who_changed_status),
			(select UC.issuer_dn from users_certificates UC where UC.cert_id = S.who_changed_status),
			(select L.args as args
					from log_system_tasks L
					where L.task_id = S.task_id and L.action_type_id = S.status_task
					order by L.id desc limit 1)
		from system_tasks S
		inner join ref_type_task TT on TT.id = S.task_type_id
		where S.task_id = task_id_in;
end;
$$
language plpgsql;