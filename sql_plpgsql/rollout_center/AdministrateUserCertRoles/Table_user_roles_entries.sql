-- Table: user_roles_entries

DROP TABLE user_roles_entries;

CREATE TABLE user_roles_entries
(
  user_cert_id bigint NOT NULL,
  role_id bigint NOT NULL,
  CONSTRAINT user_roles_entries_user_cert_id_fkey FOREIGN KEY (user_cert_id)
      REFERENCES users_certificates (cert_id) MATCH FULL
      ON UPDATE NO ACTION ON DELETE CASCADE,
  CONSTRAINT user_roles_entries_user_role_fkey FOREIGN KEY (role_id)
      REFERENCES roles (role_id) MATCH FULL
      ON UPDATE NO ACTION ON DELETE CASCADE
)
WITH (
  OIDS=FALSE
);
ALTER TABLE user_roles_entries
  OWNER TO postgres;

ALTER TABLE user_roles_entries ADD CONSTRAINT pk_user_roles_entries_pk primary key (user_cert_id, role_id);