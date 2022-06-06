-- function process_next_event();

DROP FUNCTION process_next_event();

create or replace function process_next_event()
returns table(msg_id bigint) as
$$
declare
	subject_id bigint; -- ID субъекта, выполняющего обработку события
	cur_msg_id bigint; -- ID последней обработанной записи в журнале journal_event
	id_first_not_proc bigint; -- ID первой не обработанной записи в журнале journal_event
	cur_event_type_id bigint; -- ID типа события для текущей необработанной записи в журнале событий
	cur_object_type_id bigint; -- ID объекта события для текущей необработанной записи в журнале событий
	cur_display_name_object_id varchar; -- display_name для объекта события
	cur_event_result boolean; -- результат события для текущей необработанной записи
	msg_id_exists bigint; -- ID сообщения уже зарегистрированного в таблице состояний
	state_category bigint; -- Категория события
begin
	-- Получаем ID субъекта из временной таблицы tt_subject_id
	select into subject_id get_current_user_cert_id();
	
	-- Получаем ID последней обработанной записи
	select into cur_msg_id MTSI.msg_id from current_msg_id_entry MTSI;

	if cur_msg_id is null then -- Если метки msg_id нет, то создаем метку и ставим id в 0
		cur_msg_id := 0;
		insert into current_msg_id_entry(msg_id) values(cur_msg_id);
	end if;

	-- Получаем первую запись в журнале, у которой id больше заданной метки
	select into id_first_not_proc JE.id from journal_event JE where JE.id > cur_msg_id order by JE.id asc limit 1;

	-- Если все записи обработаны, возвращаем null
	if id_first_not_proc is null then
		return;
	end if;

	-- Получаем ID типа события, ID объекта события, результат события для текущей необработанной записи в журнале событий
	select JE.event_type_id, JE.object_id
	from journal_event JE
	where JE.id = id_first_not_proc
	into cur_event_type_id, cur_object_type_id, cur_event_result;

	-- Получаем категорию состояния события(зеленый, желтый, красный)
	select into state_category RTE.state from ref_types_events RTE where RTE.event_type_id = cur_event_type_id;

	-- Если типа события или объекта не существует, либо событие неуспешно, обновляем метку ID и возвращаем null
	if cur_event_type_id is null or cur_object_type_id is null or not cur_event_result then
		update current_msg_id_entry set msg_id = id_first_not_proc;
		return;
	end if;

	-- Получаем отображаемое имя объекта текущей необработанной записи в журнале событий
	select into cur_display_name_object_id get_display_name_by_id(cur_object_type_id);

	if state_category > 1 then -- Если событие не зеленое, то добавляем в таблицу причин состояний
		-- Проверяем наличие уже зарегистрированного события с ключевой парой event_type_id -> object_id
		select into msg_id_exists USR.msg_id from journal_event JE
				inner join user_state_reasons USR on USR.event_type_id = JE.event_type_id
				and USR.object_id = JE.object_id
				where JE.id = id_first_not_proc;

		if not msg_id_exists is null then
			-- Если такое событие уже существует, обновляем время последнего зарегистрированного события и количество
			update user_state_reasons set msg_id = id_first_not_proc, when_reg_last = now(), count = count + 1
			where event_type_id = cur_event_type_id and object_id = cur_object_type_id;
		else
			-- Добавляем новое событие в таблицу состояний для пользователя
			insert into user_state_reasons(msg_id, event_type_id, object_id, display_name_object)
			values(id_first_not_proc, cur_event_type_id, cur_object_type_id, cur_display_name_object_id);
		end if;
	end if;

	-- Удаляем из таблиц состояний то событие, которое может быть погашено текущим событием.
	delete from user_state_reasons USR
	where USR.object_id = cur_object_type_id
		and USR.event_type_id in (
			select RED.idevent
			from ref_events_dependencies RED
			where RED.id_who_relieve = cur_event_type_id);

	-- Обновляем метку последнего обработанного события
	update current_msg_id_entry set msg_id = id_first_not_proc;

	-- Возвращаем ID последнего обработанного события
	return query select id_first_not_proc;
end;
$$
language plpgsql;