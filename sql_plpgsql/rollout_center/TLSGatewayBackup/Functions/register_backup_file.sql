-- FUNCTION register_backup_file(character varying,character varying,bigint,timestamp with time zone,bigint,character varying);

DROP FUNCTION register_backup_file(character varying,character varying,timestamp with time zone,bigint,character varying);

create or replace function register_backup_file(
	IN directory_in varchar,
	IN fname_in varchar,
	IN when_created_in timestamptz,
	IN size_backup_in bigint,
	IN comment_in varchar default null,
	OUT id_file_out bigint) as
$$
declare
	subject_id_ bigint;
begin
	-- Получаем ID текущего пользователя
	select into subject_id_ get_current_user_cert_id();

	if ( directory_in is null or fname_in is null or size_backup_in is null or when_created_in is null )
	then
		execute 'select add_last_error(-6);'; -- NULL недопустим.
		return;
	end if;

	if exists (select * from tlsgtw_backups TB where TB.fname = fname_in)
	then
		execute 'select add_last_error(-15);'; -- Объект должен иметь уникальное имя.
		return;
	end if;

	if not check_permission_to_object_type(subject_id_, 'backup_file', null, 'w')
	then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	insert into tlsgtw_backups(dir, fname, when_created, commentary, file_size, id_who_registered)
	values(directory_in, fname_in, when_created_in, comment_in, size_backup_in, subject_id_)
	returning id into id_file_out;
end;
$$ language plpgsql;