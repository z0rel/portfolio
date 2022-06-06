-- Посчитать формат конструкции
create or replace procedure api_construction_side__do_count_of_formats_construction(
    format_id_val bigint,
    construction_id_val bigint,
    incdec_val int
) as
$$
declare
    res_upd int := null;
begin
    if format_id_val is not NULL and construction_id_val is not NULL and incdec_val is not NULL then
        if incdec_val < 0 then
            -- Если нужно уменьшить счетчик, то не проверяем, существует ли он в таблице
            -- Поскольку возможно потом придется удалять нулевые счетчики - установим сериализуемый уровень транзации
            -- TODO: в триггерной процедуре запрещены команды управления транзакциями.
            --   следующие два оператора должны выполняться SERIALIZABLE. Вероятно нужно будет переделывать дизайн
            --   вызова команд на модификации сторон конструкций в Django для сериализации этого нюанса
            --   на установку уровня транзакции SERIALIZABLE
            update api_constructionformats
            set count = count + incdec_val
            where format_id = format_id_val
              and construction_id = construction_id_val
            returning count into res_upd;

            -- Удалять нулевые счетчики, если они существуют
            if res_upd is not null and res_upd = 0 then
                delete from api_constructionformats
                where format_id = format_id_val and construction_id = construction_id_val;
            end if;

        elsif incdec_val > 0 then
            -- Если нужно увеличить счетчик, делаем обновление с контролем существования счетчика в таблице
            insert into api_constructionformats (count, construction_id, format_id)
            values (incdec_val, construction_id_val, format_id_val)
            on conflict (construction_id, format_id) do update set count = api_constructionformats.count + incdec_val;

        end if;
    end if;
end;
$$ language plpgsql;
