-- Table: resources_requests_entries

DROP TABLE resources_requests_entries;

CREATE TABLE resources_requests_entries
(
	user_cert_id bigint NOT NULL,
	requested_resource_id bigint,

	CONSTRAINT resources_requests_entries_user_cert_id_fkey FOREIGN KEY (user_cert_id)
			REFERENCES users_certificates (cert_id) MATCH FULL
			ON UPDATE NO ACTION ON DELETE CASCADE,

	CONSTRAINT resources_requests_entries_resource_id_fkey FOREIGN KEY (requested_resource_id)
			REFERENCES proxy_resources (resource_id) MATCH FULL
			ON UPDATE NO ACTION ON DELETE CASCADE,

	CONSTRAINT resources_requests_entries_pk PRIMARY KEY (user_cert_id, requested_resource_id)
)
WITH (
	OIDS=FALSE
);
ALTER TABLE resources_requests_entries
	OWNER TO postgres;