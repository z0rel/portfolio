-- table permission_role_event

drop table permission_role_event;

CREATE TABLE permission_role_event
(
  role_id bigint NOT NULL,
  ev_type_id bigint NOT NULL,
  CONSTRAINT permission_role_events_role_id_fkey FOREIGN KEY (role_id)
      REFERENCES roles (role_id) MATCH FULL
      ON UPDATE NO ACTION ON DELETE CASCADE,
  CONSTRAINT permission_role_events_event_type_id_fkey FOREIGN KEY (ev_type_id)
      REFERENCES ref_types_events (event_type_id) MATCH FULL
      ON UPDATE NO ACTION ON DELETE CASCADE,
  CONSTRAINT uq_permission_role_events_key UNIQUE (role_id, ev_type_id)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE permission_role_event
  OWNER TO postgres;