--function list_backup_files();

DROP FUNCTION list_backup_files();

create or replace function list_backup_files()
returns table(
	id bigint,
	dir varchar,
	fname varchar,
	when_created timestamptz,
	comment varchar,
	file_size bigint,
	when_registered timestamptz,
	id_who_registered bigint,
	type_who_registered varchar,
	display_name_who_registered varchar
) as
$$
declare
	subject_id_ bigint;
begin
	-- Получаем ID текущего пользователя
	select into subject_id_ get_current_user_cert_id();

	if not check_permission_to_object_type(subject_id_, 'backup_file', null, 'r')
	then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	return query select
			TB.id,
			TB.dir,
			TB.fname,
			cast(TB.when_created at time zone 'utc' as timestamptz),
			TB.commentary,
			TB.file_size,
			cast(TB.when_registered at time zone 'utc' as timestamptz),
			TB.id_who_registered,
			(select get_type_object_by_id(TB.id_who_registered, null)),
			(select get_display_name_by_id(TB.id_who_registered))
		from tlsgtw_backups TB;
end;
$$ language plpgsql;