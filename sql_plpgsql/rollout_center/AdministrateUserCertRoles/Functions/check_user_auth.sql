-- Function: check_user_auth(role_group)

drop function check_user_auth(role_group);

create or replace function check_user_auth(role_group_type role_group)
  returns table (
	subject_id bigint,
  subject_cn varchar(64),
	cert_roles_ids varchar[]) as
$$
declare
	subject_id_ bigint;

	subject_roles_ varchar[];
begin
	-- Получаем ID текущего пользователя
	select into subject_id_ get_current_user_cert_id();

	select array(select R.string_id
								from roles R
								inner join user_roles_entries URE on R.role_id = URE.role_id
								where R.group_type = role_group_type and URE.user_cert_id = subject_id_)
	into subject_roles_;

	if (subject_roles_ is null)
	then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	elseif (role_group_type = 'user' and subject_roles_ && array['bu']::varchar[]) then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

  return query select
  UC.cert_id,
  UC.subject_cn,
  subject_roles_
  from users_certificates UC
  where UC.cert_id = subject_id_;
end;
$$
language plpgsql;