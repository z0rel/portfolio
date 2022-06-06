-- function check_permission_to_object_type(bigint, varchar, varchar, permission_type)

drop function check_permission_to_object_type(bigint, varchar, varchar, permission_type);

create or replace function check_permission_to_object_type(subject_cert_id_in bigint, object_type_in varchar, object_name_in varchar, permission permission_type) -- 'r', 'w'
returns boolean as
$$
declare
	subject_roles_ varchar[];
begin
	if (subject_cert_id_in is null or object_type_in is null) then
		execute 'select add_last_error(-6);'; -- NULL недопустим.
		return false;
	end if;

	if not exists (select ObjTypes.local_name from ref_object_types ObjTypes
									where ObjTypes.local_name = object_type_in)
									and not (object_type_in = any (array['certificate', 'admin_certificate', 'user_certificate', 'blocked_user_certificate']::varchar[])) then
		execute 'select add_last_error(-13);'; -- Объекта с данным строковым идентификатором нет.
		return false;
	end if;

	if (subject_cert_id_in = 17179869184) then
		return true; -- tls gateway system identifier
	end if;

	select array(select R.string_id from roles R inner join user_roles_entries URE on R.role_id = URE.role_id
	where URE.user_cert_id = subject_cert_id_in)
	into subject_roles_;

	case
		when (object_type_in = 'system') then
			return subject_roles_ && array['seca']::varchar[];

		when (object_type_in = any (array['certificate', 'admin_certificate', 'user_certificate', 'blocked_user_certificate']::varchar[])) then
			if (permission = 'r') then
				return subject_roles_ && array['seca', 'a', 'aud']::varchar[];
			else
				-- АБ может управлять ролью администраторов, администраторов безопасности и аудиторов
				return ((subject_roles_ && array['seca']::varchar[]) and (object_type_in = any (array['certificate', 'admin_certificate']::varchar[])))
				-- Администратор может управлять пользователями
				or ((subject_roles_ && array['a']::varchar[]) and (object_type_in = any (array['user_certificate', 'blocked_user_certificate']::varchar[])));
			end if;

		when (object_type_in = 'role') then
			if (permission = 'r') then
				return subject_roles_ && array['seca', 'a', 'aud']::varchar[];
			else
				-- АБ может управлять ролью администраторов, администраторов безопасности и аудиторов
				return ((subject_roles_ && array['seca']::varchar[]) and (object_name_in = any (array['a', 'seca', 'aud']::varchar[])))
				-- Администратор может управлять пользователями
				or ((subject_roles_ && array['a']::varchar[]) and (object_name_in = any (array['u', 'bu']::varchar[])));
			end if;

		when (object_type_in = 'proxy_resource') then
			if (permission = 'r') then
				return (subject_roles_ && array['seca', 'a', 'aud']::varchar[]) or
							 (subject_roles_ && array['u']::varchar[] and not (subject_roles_ && array['bu']::varchar[]));
			else
				return subject_roles_ && array['a']::varchar[];
			end if;

		when (object_type_in = 'ca_certificate') then
			return (subject_roles_ && array['seca']::varchar[]);

		when (object_type_in = 'event_type') then
				return check_permission_to_event(subject_cert_id_in, (select RTE.event_type_id from ref_types_events RTE where RTE.local_name = object_name_in));

		when (object_type_in = 'backup_file') then
			return (subject_roles_ && array['a']::varchar[]);

		else
			return false;
	end case;
end;
$$
language plpgsql;