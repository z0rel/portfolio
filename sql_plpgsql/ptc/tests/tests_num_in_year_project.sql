DROP trigger if exists api_num_in_year_project on api_project RESTRICT;
DROP function api_num_in_year_project();


insert into api_project (title, agency_commission, brand_id, client_id,
                         creator_id, back_office_manager_id, sales_manager_id,
                         start_date, is_archive)
                         values ('Тест1', false, 4, 51, 8, 2, 6,
                                 '2019-06-07 22:00:00.000000', false),
                                ('Тест2', false, 4, 51, 8, 2, 6,
                                 '2019-06-07 22:00:00.000000', false),
                                ('Тест3', false, 4, 51, 8, 2, 6,
                                 '2020-06-07 22:00:00.000000', false),
                                ('Тест4', false, 4, 51, 8, 2, 6,
                                 '2020-06-07 22:00:00.000000', false),
                                ('Тест5', false, 4, 51, 8, 2, 6,
                                 '2021-06-07 22:00:00.000000', false),
                                ('Тест6', false, 4, 51, 8, 2, 6,
                                 '2020-06-07 22:00:00.000000', false);

insert into api_project (title, agency_commission, brand_id, client_id,
                         creator_id, back_office_manager_id, sales_manager_id,
                         is_archive) values ('Тест6', false, 4, 51, 8, 2, 6, false); -- Exception
