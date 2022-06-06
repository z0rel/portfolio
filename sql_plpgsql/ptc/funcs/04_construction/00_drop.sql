drop trigger if exists api_construction_integrity_family on api_construction restrict;
drop trigger if exists api_construction_num_in_district on api_construction restrict;
drop trigger if exists api_construction_status_connection on api_construction restrict;
drop function if exists api_construction_status_connection() cascade;
drop function if exists api_construction_num_in_district() cascade;
drop function if exists api_construction_integrity_family() cascade;