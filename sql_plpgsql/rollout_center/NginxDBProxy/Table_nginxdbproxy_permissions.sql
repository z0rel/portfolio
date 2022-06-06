-- Table: nginxdb_proxy_permissions

drop table nginxdb_proxy_permissions;

CREATE TABLE nginxdb_proxy_permissions
(
	iid int NOT NULL,
	rid int NOT NULL,
	cert_serial_number varchar NOT NULL,

	CONSTRAINT pk_nginxdb_proxy_permission PRIMARY KEY (iid, rid, cert_serial_number) USING INDEX TABLESPACE nginxdbproxy_index
)
WITH (OIDS=FALSE)
TABLESPACE nginxdbproxy_data;
ALTER TABLE user_resources_access_entries
	OWNER TO postgres;