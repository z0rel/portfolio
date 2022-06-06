-- Function: auto_check_license_exp_date()

DROP FUNCTION auto_check_license_exp_date();

CREATE OR REPLACE FUNCTION auto_check_license_exp_date()
	RETURNS void AS
$$
declare
	subject_id_ bigint;

	time_now_ timestamp with time zone;
	license_expire_date_ timestamptz;
	license_exp_days_event_ bigint;
	license_max_users_limit_ bigint;
	license_id_ varchar;
	current_users_ bigint;
begin
	-- Получаем ID текущего сертификата пользователя
	select into subject_id_ get_current_user_cert_id();

	select into time_now_ now();
	select get_string_tlsgtw_option('license', 'expiration_date')::timestamptz into license_expire_date_;
	select get_int_tlsgtw_option('license', 'check_expire_in_days') into license_exp_days_event_;
	select get_int_tlsgtw_option('license', 'max_users') into license_max_users_limit_;
	select get_string_tlsgtw_option('license', 'id') into license_id_;

	if (license_exp_days_event_ is null) or (license_expire_date_ is null) or (license_max_users_limit_ is null)
	then
		execute 'select add_last_error(-32);'; -- Лицензия не загружена
		return;
	end if;

	select count(*) from user_roles_entries URE
	inner join roles R on R.role_id = URE.role_id
	where R.string_id = 'u' into current_users_;

	if (current_users_ >= license_max_users_limit_ * 0.95)
	then
		-- событие: количество пользователей приближается к максимальному
		perform register_event('license_users_count_expiring', (select SO.obj_id from system_objects SO where SO.local_name = 'tlsgtw'),null, true, array[license_id_]::varchar[]);
	end if;

	if (select extract(day from license_expire_date_ - time_now_) < license_exp_days_event_)
	then
		-- если срок действия истек
		if ( extract(second from license_expire_date_ - time_now_) < 0 )
		then
			-- событие: лицензия истекла
			perform register_event('license_expired', (select SO.obj_id from system_objects SO where SO.local_name = 'tlsgtw'), null, true, array[license_id_]::varchar[]);

		else
			-- до конца действия меньше чем check_expire_in_days дней
			perform register_event('license_expiring', (select SO.obj_id from system_objects SO where SO.local_name = 'tlsgtw'), null, true, array[license_id_]::varchar[]);
		end if;
	end if;

	return;
end;
$$
	language plpgsql volatile

