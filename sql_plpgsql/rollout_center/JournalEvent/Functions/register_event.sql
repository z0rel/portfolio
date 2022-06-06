-- fucntion register_event(varchar, bigint, varchar, boolean, varchar[])

drop function register_event(varchar, bigint, varchar, boolean, varchar[]);

create or replace function register_event(event_type_in varchar, object_id_in bigint, object_role_type_in varchar, result_in boolean, args_in varchar[])
returns bigint as
$$
declare
	subject_id_ bigint; -- ID субъекта, выполняющего данную функцию
	ev_type_id_ bigint; -- ID типа события

	ret_id_ bigint; -- ID  зарегистрированного события
begin
	-- Получаем ID субъекта из временной таблицы tt_subject_id
	select into subject_id_ get_current_user_cert_id();

	-- Получаем ID типа события по его внутреннему имени
	select  event_type_id from ref_types_events where local_name = event_type_in into ev_type_id_;

	-- Если ID типа события или ID объекта равны null, то возвращаем null, иначе регистрируем событие
	if not (ev_type_id_ is null) then
		insert into journal_event(event_type_id, subject_id, subject_role_group, object_id, object_role_group, result, args)
		values
    (
			ev_type_id_,
			subject_id_,
			null,
			object_id_in,
			object_role_type_in::role_group,
			result_in,
			args_in
		)
		returning id into ret_id_;
	else
		select into ret_id_ null;
	end if;

	-- Возвращаем null, если произошла ошибка. Иначе возвращаем ID зарегистрированного события.
	return ret_id_;
end;
$$
language plpgsql;