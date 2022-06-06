-- Обновить счетчики форматов конструкции при создании, обновлении или удалении стороны
create or replace function api_construction_side__count_owner_format() returns trigger as
$$
declare
    format_id__old integer;
    format_id__new integer;
    -- old            api_constructionside%rowtype;
    -- new            api_constructionside%rowtype;
begin
    if (tg_op = 'UPDATE') then
        -- Формат не изменился: если id конструкции-владельца не изменилось И
        if (new.construction_id is NULL and old.construction_id is null) or
           (new.construction_id = old.construction_id) then
            -- И не изменилось id типа рекламной стороны
            if ((new.advertising_side_id is NULL and old.advertising_side_id is NULL)
                or (new.advertising_side_id = old.advertising_side_id)) then
                return new; -- вернуть неизмененную сторону конструкции
            end if;
            -- или

            -- выбрать старый id формата
            format_id__old := api_construction_side__get_format_id(old.advertising_side_id);
            -- выбрать новый id формата
            format_id__new := api_construction_side__get_format_id(new.advertising_side_id);

            -- Если id формата не изменился
            if ((format_id__new is NULL and format_id__old is NULL)
                or (format_id__new = format_id__new)) then
                return new; -- вернуть неизмененную сторону конструкции
            end if;

            call api_construction_side__do_count_of_formats_construction(
                    format_id__old,
                    old.construction_id,
                    -1
                );
            call api_construction_side__do_count_of_formats_construction(
                    format_id__new,
                    new.construction_id,
                    1
                );
        else -- Обработка смены формата при смене конструкции-владельца
            format_id__old := api_construction_side__get_format_id(old.advertising_side_id);
            format_id__new := api_construction_side__get_format_id(new.advertising_side_id);

            call api_construction_side__do_count_of_formats_construction(
                    format_id__old,
                    old.construction_id,
                    -1
                );
            call api_construction_side__do_count_of_formats_construction(
                    format_id__new,
                    new.construction_id,
                    1
                );
        end if;
        return new;
    elsif (tg_op = 'INSERT') then
        format_id__new := api_construction_side__get_format_id(new.advertising_side_id);
        call api_construction_side__do_count_of_formats_construction(
                format_id__new,
                new.construction_id,
                1
            );
        return new;
    elsif (tg_op = 'DELETE') then
        format_id__old := api_construction_side__get_format_id(old.advertising_side_id);
        call api_construction_side__do_count_of_formats_construction(
                format_id__old,
                old.construction_id,
                -1
            );
        return old;
    end if;
    return new;
end;
$$ language plpgsql;
