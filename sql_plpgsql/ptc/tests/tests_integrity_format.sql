DROP trigger if exists api_construction_side_integrity_format on api_constructionside RESTRICT;
DROP function api_construction_side_integrity_format();

update api_constructionside set advertising_side_id = 1086 where id=2; -- OK (format_id = 73, 73)
update api_constructionside set advertising_side_id = 1070 where id=2; -- EXCEPTION (format_id = 71, 73)
update api_constructionside set construction_id = 73 where id=1; -- OK (format_id= 73, 73)
update api_constructionside set construction_id = 123 where id=1; -- EXCEPTION (format_id = 75, 73)

insert into api_constructionside (availability_side, advertising_side_id, construction_id)
    values (true, 1086, 36); --OK

insert into api_constructionside (availability_side, advertising_side_id, construction_id)
    values (true, 1085, 123); --EXCEPTION

