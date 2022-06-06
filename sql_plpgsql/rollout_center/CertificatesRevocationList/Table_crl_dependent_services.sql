-- table crl_dependent_services

drop table crl_dependent_services;

create table crl_dependent_services(
	service_name varchar NOT NULL,
	state bigint,

	CONSTRAINT pk_crl_dependent_services PRIMARY KEY (service_name)
);

insert into crl_dependent_services (service_name, state)
values ('https_access2_server', 1);

insert into crl_dependent_services (service_name, state)
values ('https_access1_server', 1);

insert into crl_dependent_services (service_name, state)
values ('https_control_server', 1);