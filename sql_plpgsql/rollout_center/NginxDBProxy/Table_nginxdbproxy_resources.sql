-- Table: nginxdb_proxy_resource

drop table nginxdb_proxy_resources cascade;

CREATE TABLE nginxdb_proxy_resources
(
	rid serial NOT NULL,
	external_addr varchar NOT NULL,

	CONSTRAINT pk_nginxdb_proxy_resource PRIMARY KEY (external_addr) USING INDEX TABLESPACE nginxdbproxy_index
)
WITH (OIDS=FALSE)
TABLESPACE nginxdbproxy_data;
ALTER TABLE nginxdb_proxy_resources
	OWNER TO postgres;