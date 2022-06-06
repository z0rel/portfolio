-- Function: edit_user_resources_request(varchar, varchar, bigint[])

DROP FUNCTION edit_user_resources_request(varchar, varchar, bigint[]);

CREATE OR REPLACE FUNCTION edit_user_resources_request(
	IN request_description_in varchar,
	IN user_email_in varchar,
	IN resources_in bigint[]) returns void as
$BODY$
declare
	subject_id_ bigint;
	last_modify_date_ timestamptz;
	resource_id_ bigint;
begin
	-- Получаем ID текущего сертификата пользователя
	select into subject_id_ get_current_user_cert_id();

	if not check_permission_to_object_type(subject_id_, 'proxy_resource', null, 'r')
	then
		execute 'select add_last_error(-1);'; -- Нет прав на выполнение данного действия с данным объектом.
		return;
	end if;

	if not exists (select * from resources_access_requests RAR where RAR.user_cert_id = subject_id_)
	then
		insert into resources_access_requests (user_cert_id, request_description, user_email)
		values (subject_id_, request_description_in, user_email_in);
	else
		update resources_access_requests set
		request_description = request_description_in,
		user_email = user_email_in,
		request_date = now()
		where user_cert_id = subject_id_;
	end if;

	-- обновляем список запрошенных ресурсов
	delete from resources_requests_entries RRE where RRE.user_cert_id = subject_id_;
	foreach resource_id_ in array resources_in
	loop
		insert into resources_requests_entries (user_cert_id, requested_resource_id) values (subject_id_, resource_id_);
	end loop;

end;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION edit_user_resources_request(varchar, varchar, bigint[])
  OWNER TO postgres;
COMMENT ON FUNCTION edit_user_resources_request(varchar, varchar, bigint[]) IS 'Добавление или редактирование запросов пользователя на ресурс.';
