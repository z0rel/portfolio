-- Function: get_all_id_roles_for_user(bigint)

DROP FUNCTION get_all_id_roles_for_user(bigint);

CREATE OR REPLACE FUNCTION get_all_id_roles_for_user(id_user_in bigint)
  RETURNS setof bigint AS
$BODY$
declare
  subject_id_ bigint;
begin
  	-- Получаем ID текущего пользователя
	select into subject_id_ get_current_user_cert_id();

	if not check_permission_to_object_type(subject_id_, 'certificate', null, 'r') then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	  if not check_permission_to_object_type(subject_id_, 'role', null, 'r') then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	return query select user_roles_entries.role_id
		from user_roles_entries
		where user_roles_entries.user_cert_id = id_user_in;
end;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100
  ROWS 1000;
ALTER FUNCTION get_all_id_roles_for_user(bigint)
  OWNER TO postgres;
COMMENT ON FUNCTION get_all_id_roles_for_user(bigint) IS 'Возвращает список ID ролей, в состав которых входит пользователь';