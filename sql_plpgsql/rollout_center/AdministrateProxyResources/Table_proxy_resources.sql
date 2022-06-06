-- Table: proxy_resources

DROP TYPE tls_type CASCADE;
CREATE TYPE tls_type AS ENUM ('oneSideTLS', 'twoSideTLS');

DROP TABLE proxy_resources CASCADE;
CREATE TABLE proxy_resources
(
	resource_id bigint NOT NULL DEFAULT nextval('proxy_resources_id_seq'::regclass),
	resource_name character varying NOT NULL,
	resource_description character varying,
	internal_address character varying NOT NULL,
	external_address character varying NOT NULL,
	auth_mode tls_type,

	last_modify_date timestamp with time zone default now(),

	CONSTRAINT pk_proxy_resource PRIMARY KEY (external_address),
	CONSTRAINT unq_proxy_resource_id UNIQUE  (resource_id)
)
WITH (
	OIDS=FALSE
);
ALTER TABLE proxy_resources
	OWNER TO postgres;
COMMENT ON TABLE proxy_resources
	IS 'Таблица содержит информацию о проксируемых ресурсах';

alter table proxy_resources add column nginxdb_proxy_subfilter varchar default null;
alter table proxy_resources alter column auth_mode set not null;