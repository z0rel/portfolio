update api_construction set code = 213 where id=3 returning num_in_district;
update api_construction set code = 214 where id=4 returning num_in_district;
update api_construction set code = 215 where id=5 returning num_in_district;
update api_construction set code = 215 where id=3 returning num_in_district;
update api_construction set code = 216 where id=4 returning num_in_district;
update api_construction set code = 217 where id=5 returning num_in_district;

insert into api_construction (code, coordinates, tech_invent_number, buh_invent_number,
                              status_connection, active, is_nonrts, crew_id, format_id,
                              location_id)
            values (1111, '43.24117, 76.92755', '114620', '114620',
                    true, false, false, 1, 72,
                    1905) returning num_in_district;

insert into api_construction (code, coordinates, tech_invent_number, buh_invent_number,
                              status_connection, active, is_nonrts, crew_id, format_id)
            values (1111, '43.24117, 76.92755', '114621', '114621',
                    true, false, false, 1, 72) returning num_in_district;