-- Table: users_certificates

DROP TABLE users_certificates CASCADE;

CREATE TABLE users_certificates
(
	cert_id bigint NOT NULL DEFAULT nextval('users_certificates_id_seq'::regclass),
	certificate bytea NOT NULL,                 -- base64 certificate data
	certificate_ext character varying NOT NULL, -- расширение файла сертификата

	serial_number character varying NOT NULL,
	issuer_dn character varying NOT NULL,       -- distinguished name - отличительное имя издателя
	issuer_cn character varying(64) NOT NULL,   -- общее имя издателя
	issuer_key_identifier bytea NOT NULL,       -- идентификатор ключа издателя

	subject_cn character varying(64) NOT NULL,  -- общее имя субъекта
	subject_o character varying(1024),          -- организация
	subject_snils character(11),                -- СНИЛС
	subject_surname character varying(40),      -- фамилия
	subject_givenname character varying(64),    -- имя и отчество
	subject_t character varying(64),            -- должность
	subject_ou character varying(64),           -- подразделение
	subject_street character varying(30),       -- улица
	subject_l character varying(128),           -- город
	subject_s character varying(128),           -- код субъекта РФ согласно классификатору КЛАДР
	subject_c character(2),                     -- двухсимвольный код страны
	subject_email character varying(255),       -- адрес электронной почты
	subject_inn character varying(12),          -- ИНН
	subject_ogrn character(13),                 -- ОГРН
	valid_from timestamp with time zone NOT NULL,
	valid_to timestamp with time zone NOT NULL,
	commentary character varying,

	last_permissions_modify_date timestamp with time zone default now(),

	CONSTRAINT pk_user_cert PRIMARY KEY (issuer_dn, serial_number),
	CONSTRAINT unq_cert_id UNIQUE (cert_id)
)
WITH (
	OIDS=FALSE
);
ALTER TABLE users_certificates
	OWNER TO postgres;
COMMENT ON TABLE users_certificates
	IS 'Таблица содержит информацию о сертификатах пользователей.';
