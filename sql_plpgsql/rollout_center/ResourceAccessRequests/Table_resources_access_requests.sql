-- Table: resources_access_requests

DROP TABLE resources_access_requests;

CREATE TABLE resources_access_requests
(
	user_cert_id bigint NOT NULL,

	request_description varchar,
	user_email varchar,
	request_date timestamp with time zone default now(),

	CONSTRAINT resources_access_requests_user_cert_id_fkey FOREIGN KEY (user_cert_id)
			REFERENCES users_certificates (cert_id) MATCH FULL
			ON UPDATE NO ACTION ON DELETE CASCADE,

	CONSTRAINT resources_access_requests_pk PRIMARY KEY (user_cert_id)
)
WITH (
	OIDS=FALSE
);
ALTER TABLE resources_access_requests
	OWNER TO postgres;