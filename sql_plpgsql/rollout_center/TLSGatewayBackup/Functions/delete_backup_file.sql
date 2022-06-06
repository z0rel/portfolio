--function delete_backup_file( bigint );

drop function delete_backup_file( bigint );

create or replace function delete_backup_file( id_in bigint )
returns boolean as
$$
declare
	subject_id_ bigint;
begin
	-- Получаем ID текущего пользователя
	select into subject_id_ get_current_user_cert_id();

	if not check_permission_to_object_type(subject_id_, 'backup_file', null, 'w')
	then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return false;
	end if;

	delete from tlsgtw_backups TB
	where TB.id = id_in;
	if not FOUND then
		execute 'select add_last_error(3);'; -- Удаляемый объект не существует.
	end if;

	return true;
end;
$$
language plpgsql;