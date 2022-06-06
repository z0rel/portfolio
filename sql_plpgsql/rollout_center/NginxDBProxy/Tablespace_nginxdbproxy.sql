
drop tablespace nginxdbproxy_data;
drop tablespace nginxdbproxy_index;

CREATE TABLESPACE nginxdbproxy_data LOCATION '/opt/itcs/postgres_data/nginxdbproxy_data';
CREATE TABLESPACE nginxdbproxy_index LOCATION '/opt/itcs/postgres_data/nginxdbproxy_index';
