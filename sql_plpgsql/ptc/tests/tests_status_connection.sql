DROP trigger if exists api_construction_status_connection on api_construction RESTRICT;
DROP function api_construction_status_connection();

update api_construction set tech_problem = 'Нет напряжения' where id=1 returning status_connection; -- true
update api_construction set tech_problem = '', status_connection = false where id=1;

update api_construction set tech_problem = 'нет Напряжения' where id=1 returning status_connection; --true
update api_construction set tech_problem = '', status_connection = false where id=1;

update api_construction set tech_problem = 'Проблема Нет напряжения' where id=1 returning status_connection; --true
update api_construction set tech_problem = '', status_connection = false where id=1;

update api_construction set tech_problem = 'Проблема нет напряжения' where id=1 returning status_connection; --true
update api_construction set tech_problem = '', status_connection = false where id=1;

update api_construction set tech_problem = 'Не работают все диоды' where id=1 returning status_connection; --true
update api_construction set tech_problem = '', status_connection = false where id=1;

update api_construction set tech_problem = 'не Работают Все диоды' where id=1 returning status_connection; --true
update api_construction set tech_problem = '', status_connection = false where id=1;

update api_construction set tech_problem = 'Проблема не работают все диоды' where id=1 returning status_connection; --true
update api_construction set tech_problem = '', status_connection = false where id=1;

update api_construction set tech_problem = 'Проблема не Работают Все диоды' where id=1 returning status_connection; --true
update api_construction set tech_problem = '', status_connection = false where id=1;

update api_construction set tech_problem = 'Не работает двигатель' where id=1 returning status_connection; --false
update api_construction set tech_problem = 'Ремонт постеродержателя' where id=1 returning status_connection; --false
update api_construction set tech_problem = 'Замена лавочки' where id=1 returning status_connection; --false
update api_construction set tech_problem = 'ДТП' where id=1 returning status_connection; --false

--------------------------------- INSERT ---------------------------------

insert into api_construction (code, coordinates, tech_invent_number, buh_invent_number,
                              status_connection, active, is_nonrts, crew_id, format_id,
                              location_id, tech_problem)
            values (1111, '43.24117, 76.92755', '114692', '114692',
                    false, false, false, 1, 73,
                    1905, 'Нет напряжения') returning status_connection; -- true,


insert into api_construction (code, coordinates, tech_invent_number, buh_invent_number,
                              active, is_nonrts, crew_id, format_id,
                              location_id, tech_problem)
            values (1111, '43.24117, 76.92755', '114693', '114693',
                    false, false, 1, 73,
                    1905, 'нет Напряжения') returning status_connection; -- true

insert into api_construction (code, coordinates, tech_invent_number, buh_invent_number,
                              active, is_nonrts, crew_id, format_id,
                              location_id, tech_problem)
            values (1111, '43.24117, 76.92755', '114694',
                    '114694', false, false, 1, 73,
                    1905, 'Проблема нет Напряжения') returning status_connection; --true

insert into api_construction (code, coordinates, tech_invent_number, buh_invent_number,
                              status_connection, active, is_nonrts, crew_id, format_id,
                              location_id, tech_problem)
            values (1111, '43.24117, 76.92755', '114695', '114695',
                    false, false, false, 1, 73,
                    1905, 'Не работают все диоды') returning status_connection; -- true,


insert into api_construction (code, coordinates, tech_invent_number, buh_invent_number,
                              active, is_nonrts, crew_id, format_id,
                              location_id, tech_problem)
            values (1111, '43.24117, 76.92755', '114696', '114696',
                    false, false, 1, 73,
                    1905, 'не Работают все Диоды') returning status_connection; -- true

insert into api_construction (code, coordinates, tech_invent_number, buh_invent_number,
                              active, is_nonrts, crew_id, format_id,
                              location_id, tech_problem)
            values (1111, '43.24117, 76.92755', '114697', '114697',
                    false, false, 1, 73,
                    1905, 'Проблема Не работают все диоды') returning status_connection; --true

insert into api_construction (code, coordinates, tech_invent_number, buh_invent_number,
                              active, is_nonrts, crew_id, format_id,
                              location_id, tech_problem, status_connection)
            values (1111, '43.24117, 76.92755', '114698', '114698',
                    false, false, 1, 73,
                    1905, 'Не работает двигатель', false) returning status_connection; --false

insert into api_construction (code, coordinates, tech_invent_number, buh_invent_number,
                              active, is_nonrts, crew_id, format_id,
                              location_id, tech_problem, status_connection)
            values (1111, '43.24117, 76.92755', '114699', '114699',
                    false, false, 1, 73,
                    1905, 'Ремонт постеродержателя', false) returning status_connection; --false