-- table auto_functions;

drop table auto_functions;

create table auto_functions(
     name varchar not null,
     pause_in_sec bigint not null,
     description varchar,
     last_execution_ts timestamptz
);

insert into auto_functions(name, pause_in_sec, description)
     values('auto_delete_expired_users_certificates', 3600, 'Очистка просроченных пользовательских сертификатов.');

insert into auto_functions(name, pause_in_sec, description)
     values('auto_check_expired_certificates_and_crl', 24*3600, 'Функция проверки срока действия корневых сертификатов и CRL');

insert into auto_functions(name, pause_in_sec, description)
     values('auto_check_license_exp_date', 24*3600, 'Функция проверки срока действия лицензии');