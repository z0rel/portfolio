--  table root_certificate;

drop table trusted_root_certificate cascade;

CREATE TABLE trusted_root_certificate(

	root_cert_id bigint NOT NULL DEFAULT nextval('root_certificate_id_seq'::regclass),

	subject_dn varchar NOT NULL,									-- имя УЦ (для самоподписанного совпадает с issuer_dn)
	subject_key_identifier bytea NOT NULL,				-- идентификатор ключа корневого сертификата
	cert_serial varchar NOT NULL,									-- серийный номер
	root_cert varchar NOT NULL,										-- root cert PEM

	issuer_dn varchar NOT NULL,										-- имя издателя корневого сертификата УЦ
	issuer_cn character varying(64) NOT NULL,			-- общее имя издателя
	subject_cn character varying(64) NOT NULL,		-- общее имя субъекта
	subject_o character varying(1024),						-- организация
	valid_from timestamp with time zone NOT NULL,
	valid_to timestamp with time zone NOT NULL,

	crl_data varchar,															-- CRL PEM
	when_created_crl timestamptz,
	when_next_crl timestamptz,
	crl_access_point varchar,
	crl_update_period_sec bigint,

	last_modify_date timestamp with time zone default now(),

	constraint pk_root_certificate primary key (subject_dn, subject_key_identifier),
	constraint pk_ca_id_unique unique (root_cert_id)
)
WITH (
	OIDS=FALSE
);
ALTER TABLE trusted_root_certificate
	OWNER TO postgres;
COMMENT ON TABLE trusted_root_certificate
	IS 'Таблица содержит информацию об доверенных корневых сертификатах';

alter table trusted_root_certificate add column is_active boolean not null default false;