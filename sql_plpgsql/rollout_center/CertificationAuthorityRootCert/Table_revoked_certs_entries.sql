--  table revoked_certs;

drop table revoked_certs cascade;

CREATE TABLE revoked_certs(

	trusted_root_cert_id bigint,

	serial_number varchar NOT NULL,

	CONSTRAINT revoked_certs_issuer_fkey FOREIGN KEY (trusted_root_cert_id)
			REFERENCES trusted_root_certificate (root_cert_id) MATCH FULL
			ON UPDATE NO ACTION ON DELETE CASCADE,

	CONSTRAINT revoked_certs_pk PRIMARY KEY (trusted_root_cert_id, serial_number)
)
WITH (
	OIDS=FALSE
);
ALTER TABLE proxy_resources
	OWNER TO postgres;
COMMENT ON TABLE proxy_resources
	IS 'Таблица содержит информацию об отозванных сертификатах';