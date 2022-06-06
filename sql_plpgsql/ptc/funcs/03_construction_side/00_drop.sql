drop trigger if exists api_construction_side_integrity_format on api_constructionside restrict;
drop function if exists api_construction_side_integrity_format() cascade;

drop procedure if exists api_construction_side__do_count_of_formats_construction(
    format_id_val bigint,
    construction_id_val bigint,
    incdec_val int
);

drop function if exists api_construction_side__get_format_id(advertising_side_id int);

drop trigger if exists api_construction_side__count_owner_format on api_constructionside restrict;
drop function if exists api_construction_side__count_owner_format() cascade;
