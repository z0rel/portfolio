-- Table: user_blocked_resources_entries

DROP TABLE user_resources_access_entries;

CREATE TABLE user_resources_access_entries
(
	user_cert_id bigint NOT NULL,
	resource_id bigint NOT NULL,
	access_allowed boolean,
	CONSTRAINT user_blocked_resources_entries_user_cert_id_fkey FOREIGN KEY (user_cert_id)
			REFERENCES users_certificates (cert_id) MATCH FULL
			ON UPDATE NO ACTION ON DELETE CASCADE,
	CONSTRAINT user_blocked_resources_entries_resource_id_fkey FOREIGN KEY (resource_id)
			REFERENCES proxy_resources (resource_id) MATCH FULL
			ON UPDATE NO ACTION ON DELETE CASCADE,


	CONSTRAINT pk_user_resources_access_entries_pk PRIMARY KEY (user_cert_id, resource_id)
)
WITH (
	OIDS=FALSE
);
ALTER TABLE user_resources_access_entries
	OWNER TO postgres;