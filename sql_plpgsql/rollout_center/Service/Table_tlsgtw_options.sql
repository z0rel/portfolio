-- table tlsgtw_options

drop table tlsgtw_options;

CREATE TABLE tlsgtw_options (
  namesection varchar,
  nameoption varchar,
  intvalue bigint,
  strvalue varchar,
  editable bool default false,
  CONSTRAINT uq_tlsgtw_options_namesection_nameoption_key UNIQUE(namesection, nameoption)
);

-- init section
insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('init', 'first_unused_slot', null, null, true);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('init', 'done', 0, null, true);


-- license cestion
insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('license', 'comment', null, null, true);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('license', 'creation_date', null, null, true);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('license', 'customer_name', null, null, true);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('license', 'id', null, null, true);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('license', 'is_present', 0, null, true);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('license', 'issuer_name', null, null, true);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('license', 'min_version', null, null, true);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('license', 'max_version', null, null, true);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('license', 'max_users', null, null, true);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('license', 'expiration_date', null, null, true);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('license', 'check_expire_in_days', 60, null, false);

-- system section
insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'appliance_serial_number', null, '0000-0000-0000-0000', false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'first_reserved_slot', 1000, null, false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'last_reserved_slot', 1299, null, false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'slot_for_crl_robot', 999, null, false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'slot_for_check_robot', 998, null, false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'slot_for_control_request_handler', 997, null, false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'supervisor_socket', null, '/tmp/tlsgatewaySupervisorSocket', false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'check_period_in_cycles', 2040109465, null, true);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'max_record_in_cache', 50000, null, true);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'revoked_sn_reg_mat_at_once', 10, null, true);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'event_auto_process_max_at_once', 10, null, false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'event_auto_process_big_pause_in_ms', 1000, null, false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'event_auto_process_small_pause_in_ms', 50, null, false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'event_auto_process_max_attemps', 1000, null, false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'event_auto_rotation_green_max_qtty', 10000000, null, false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'event_auto_rotation_keep_in_days', 10, null, false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'event_auto_rotation_yellow_max_qtty', 90000000, null, false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'event_auto_rotation_red_max_qtty', 100000000, null, false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'event_auto_rotation_small_pause_in_ms', 50, null, false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'event_auto_rotation_big_pause_in_sec', 3600, null, false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'event_auto_rotation_max_attemps', 100, null, false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'simple_auto_action_big_pause_in_ms', 10000, null, false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'simple_auto_action_small_pause_in_ms', 1000, null, false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'simple_auto_action_max_attmepts', 3, null, false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'keep_user_cert_after_expire_in_days', 90, null, false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'ca_cert_and_crl_expire_in_days', 60, null, false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'front_nginx_template_path', null, '/opt/itcs/nginx/conf/templates/front_nginx.conf.template', false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'front_nginx_config_path', null, '/opt/itcs/nginx/conf/front_nginx.conf', false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'nginx_db_proxy_conf_path', null, '/opt/itcs/nginx/conf/nginx_db_proxy.conf', false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'nginx_db_proxy_template_path', null, '/opt/itcs/nginx/conf/templates/nginx_db_proxy.conf.template', false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'proxy_module_name', null, 'nginx_db_proxy', false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'crl_robot_module_name', null, 'crl_robot', false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'license_folder', null, '/etc/rollout_center/license', false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'resource_folder', null, '/opt/tlsgtw_resources', false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('system', 'crl_update_period', 1440, null, true);

-- nginx_template
insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('nginx_template', 'worker_processes', null, '4', false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('nginx_template', 'worker_open_files', null, '30000', false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('nginx_template', 'worker_connections', null, '30000', false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('nginx_template', 'path_to_error_log_front_nginx', null, '/var/log/tlsgw/front_nginx_err.log', false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('nginx_template', 'front_nginx_pid_file', null, '/var/run/front_nginx.pid', false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('nginx_template', 'path_to_access_log_front_nginx', null, '/var/log/tlsgw/front_nginx_access.log', false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('nginx_template', 'KA_timeout', null, '60', false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('nginx_template', 'trusted_certificate', null, '/opt/itcs/nginx/conf/certs/trusted.pem', false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('nginx_template', 'server_certificate', null, '/opt/itcs/nginx/conf/certs/server_cert.pem', false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('nginx_template', 'ssl_crl', null, '/opt/itcs/nginx/conf/certs/crl.pem', false);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('nginx_template', 'fastcgi_admin_port', null, '9089', true);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('nginx_template', 'listen_port_twoside_admin', null, '443', true);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('nginx_template', 'listen_port_oneside_user', null, '994', true);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('nginx_template', 'listen_port_twoside_user', null, '443', true);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('nginx_template', 'fastcgi_user_port', null, '9090', true);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('nginx_template', 'listen_port_oneside_proxy', null, '9091', true);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('nginx_template', 'listen_port_twoside_proxy', null, '9092', true);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('nginx_template', 'nginx_db_proxy_port', null, '8080', true);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('nginx_template', 'path_to_error_log_nginx_db_proxy', null, '/var/log/tlsgw/nginx_db_proxy_error.log', true);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('nginx_template', 'path_to_access_log_nginx_db_proxy', null, '/var/log/tlsgw/nginx_db_proxy_access.log', true);

insert into tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('nginx_template', 'pid_file_nginx_db_proxy', null, '/var/run/nginx_db_proxy.pid', false);

-- task manager
INSERT INTO tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('task_manager', 'num_common_thread', 1, null, false);

INSERT INTO tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('task_manager', 'queue_length', 10, null, false);

INSERT INTO tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('task_manager', 'check_interval_sec', 5, null, false);


-- backup
INSERT INTO tlsgtw_options(namesection, nameoption, intvalue, strvalue, editable)
VALUES ('backup', 'directory', null, '/opt/resources/backups', false);



