-- Table: nginxdb_proxy_issuer

drop table nginxdb_proxy_issuer cascade;

CREATE TABLE nginxdb_proxy_issuer
(
	iid serial NOT NULL,
	issuer_dn varchar NOT NULL,

	CONSTRAINT pk_nginxdb_proxy_issuer PRIMARY KEY (issuer_dn) USING INDEX TABLESPACE nginxdbproxy_index
)
WITH (OIDS=FALSE)
TABLESPACE nginxdbproxy_data;
ALTER TABLE nginxdb_proxy_issuer
	OWNER TO postgres;

