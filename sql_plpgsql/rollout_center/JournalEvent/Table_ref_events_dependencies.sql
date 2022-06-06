-- TABLE ref_events_dependencies

drop table ref_events_dependencies cascade;

create table ref_events_dependencies
(
idevent bigint, -- ID события
id_who_relieve bigint, -- ID снимающего события
CONSTRAINT ref_events_dependencies_idevent_fkey FOREIGN KEY (idevent)
      REFERENCES ref_types_events (event_type_id) MATCH FULL
      ON UPDATE NO ACTION ON DELETE CASCADE,
CONSTRAINT ref_events_dependencies_iddepend_fkey FOREIGN KEY (id_who_relieve)
      REFERENCES ref_types_events (event_type_id) MATCH FULL
      ON UPDATE NO ACTION ON DELETE CASCADE
);

-- transport keys
insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'transport_key_expiring'),
(select event_type_id from ref_types_events where local_name = 'transport_key_activated'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'transport_key_expiring'),
(select event_type_id from ref_types_events where local_name = 'transport_key_expired'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'transport_key_activated'),
(select event_type_id from ref_types_events where local_name = 'transport_key_expiring'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'transport_key_activated'),
(select event_type_id from ref_types_events where local_name = 'transport_key_expired'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'transport_key_expired'),
(select event_type_id from ref_types_events where local_name = 'transport_key_expiring'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'transport_key_expired'),
(select event_type_id from ref_types_events where local_name = 'transport_key_activated'));


-- ca root cert
insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'ca_cert_uploaded'),
(select event_type_id from ref_types_events where local_name = 'ca_cert_expiring'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'ca_cert_uploaded'),
(select event_type_id from ref_types_events where local_name = 'ca_cert_expired'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'ca_cert_uploaded'),
(select event_type_id from ref_types_events where local_name = 'ca_cert_removed'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'ca_cert_uploaded'),
(select event_type_id from ref_types_events where local_name = 'ca_cert_revoked'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'ca_cert_expiring'),
(select event_type_id from ref_types_events where local_name = 'ca_cert_uploaded'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'ca_cert_expiring'),
(select event_type_id from ref_types_events where local_name = 'ca_cert_expired'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'ca_cert_expiring'),
(select event_type_id from ref_types_events where local_name = 'ca_cert_removed'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'ca_cert_expiring'),
(select event_type_id from ref_types_events where local_name = 'ca_cert_revoked'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'ca_cert_expired'),
(select event_type_id from ref_types_events where local_name = 'ca_cert_expiring'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'ca_cert_expired'),
(select event_type_id from ref_types_events where local_name = 'ca_cert_uploaded'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'ca_cert_expired'),
(select event_type_id from ref_types_events where local_name = 'ca_cert_removed'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'ca_cert_expired'),
(select event_type_id from ref_types_events where local_name = 'ca_cert_revoked'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'ca_cert_revoked'),
(select event_type_id from ref_types_events where local_name = 'ca_cert_uploaded'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'ca_cert_revoked'),
(select event_type_id from ref_types_events where local_name = 'ca_cert_removed'));


-- ca crl
insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'ca_crl_uploaded'),
(select event_type_id from ref_types_events where local_name = 'ca_crl_expiring'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'ca_crl_uploaded'),
(select event_type_id from ref_types_events where local_name = 'ca_crl_expired'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'ca_crl_expiring'),
(select event_type_id from ref_types_events where local_name = 'ca_crl_uploaded'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'ca_crl_expiring'),
(select event_type_id from ref_types_events where local_name = 'ca_crl_expired'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'ca_crl_expired'),
(select event_type_id from ref_types_events where local_name = 'ca_crl_uploaded'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'ca_crl_expired'),
(select event_type_id from ref_types_events where local_name = 'ca_crl_expiring'));

-- system
insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'system_module_started'),
(select event_type_id from ref_types_events where local_name = 'system_module_stopped'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'system_module_started'),
(select event_type_id from ref_types_events where local_name = 'system_module_restarted'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'system_module_started'),
(select event_type_id from ref_types_events where local_name = 'system_module_unexpectedly_stopped'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'system_module_started'),
(select event_type_id from ref_types_events where local_name = 'system_module_failed_to_start'));


insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'system_module_stopped'),
(select event_type_id from ref_types_events where local_name = 'system_module_started'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'system_module_stopped'),
(select event_type_id from ref_types_events where local_name = 'system_module_restarted'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'system_module_stopped'),
(select event_type_id from ref_types_events where local_name = 'system_module_unexpectedly_stopped'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'system_module_stopped'),
(select event_type_id from ref_types_events where local_name = 'system_module_failed_to_start'));


insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'system_module_restarted'),
(select event_type_id from ref_types_events where local_name = 'system_module_stopped'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'system_module_restarted'),
(select event_type_id from ref_types_events where local_name = 'system_module_started'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'system_module_restarted'),
(select event_type_id from ref_types_events where local_name = 'system_module_unexpectedly_stopped'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'system_module_restarted'),
(select event_type_id from ref_types_events where local_name = 'system_module_failed_to_start'));


insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'system_module_unexpectedly_stopped'),
(select event_type_id from ref_types_events where local_name = 'system_module_stopped'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'system_module_unexpectedly_stopped'),
(select event_type_id from ref_types_events where local_name = 'system_module_restarted'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'system_module_unexpectedly_stopped'),
(select event_type_id from ref_types_events where local_name = 'system_module_started'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'system_module_unexpectedly_stopped'),
(select event_type_id from ref_types_events where local_name = 'system_module_failed_to_start'));


insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'system_module_failed_to_start'),
(select event_type_id from ref_types_events where local_name = 'system_module_stopped'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'system_module_failed_to_start'),
(select event_type_id from ref_types_events where local_name = 'system_module_restarted'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'system_module_failed_to_start'),
(select event_type_id from ref_types_events where local_name = 'system_module_started'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'system_module_failed_to_start'),
(select event_type_id from ref_types_events where local_name = 'system_module_unexpectedly_stopped'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'change_web_config_failed'),
(select event_type_id from ref_types_events where local_name = 'change_web_config_succeed'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'change_web_config_succeed'),
(select event_type_id from ref_types_events where local_name = 'change_web_config_failed'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'license_expired'),
(select event_type_id from ref_types_events where local_name = 'license_loaded'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'license_expiring'),
(select event_type_id from ref_types_events where local_name = 'license_loaded'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'license_expiring'),
(select event_type_id from ref_types_events where local_name = 'license_expired'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'license_loaded'),
(select event_type_id from ref_types_events where local_name = 'license_expired'));

insert into ref_events_dependencies(idevent, id_who_relieve)
values(
(select event_type_id from ref_types_events where local_name = 'license_loaded'),
(select event_type_id from ref_types_events where local_name = 'license_expiring'));

