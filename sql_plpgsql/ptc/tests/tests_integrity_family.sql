drop trigger if exists api_construction_integrity_family on api_construction restrict;
drop function api_construction_integrity_family();

update api_construction
   set code = 213
 where id = 2; -- OK
update api_construction
   set format_id = 73
 where id = 1; -- OK
update api_construction
   set format_id = 1
 where id = 4; -- EXCEPTION
update api_construction
   set location_id = 1903
 where id = 1; --OK
update api_construction
   set location_id = 1
 where id = 4; --EXCEPTION

insert into api_construction (code, coordinates, tech_invent_number, buh_invent_number,
                              status_connection, active, is_nonrts, crew_id, format_id,
                              location_id)
values (1111, '43.24117, 76.92755', '114673', '114673',
        true, false, false, 1, 73,
        1903); -- OK

insert into api_construction (code, coordinates, tech_invent_number, buh_invent_number,
                              status_connection, active, is_nonrts, crew_id, format_id,
                              location_id)
values (1111, '43.24117, 76.92755', '114674', '114674',
        true, false, false, 1, 12,
        1903); --EXCEPTION
insert into api_construction (code, coordinates, tech_invent_number, buh_invent_number,
                              status_connection, active, is_nonrts, crew_id, format_id,
                              location_id)
values (1111, '43.24117, 76.92755', '114675', '114675',
        true, false, false, 1, 73,
        1912); --OK

insert into api_construction (code, coordinates, tech_invent_number, buh_invent_number,
                              status_connection, active, is_nonrts, crew_id, format_id,
                              location_id)
values (1111, '43.24117, 76.92755', '114675', '114675',
        true, false, false, 1, 72,
        1912); --EXCEPTION