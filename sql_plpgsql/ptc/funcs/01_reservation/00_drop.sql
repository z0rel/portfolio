drop trigger if exists api_project_cities_count on api_reservation cascade;

drop procedure if exists api_project_cities_update_count_value(
    arg_construction_side_id bigint,
    arg_project_id bigint,
    count_value bigint,
    change_saled_value bigint,
    change_distributed_value bigint,
    changed_saled_count boolean, -- Изменился ли статус "Продано"
    changed_distributed_count boolean,
    is_delete boolean) cascade;

drop function if exists api_reservation__project_cities_count() cascade;
