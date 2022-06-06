-- левое не null, правое null
create or replace function left_notnull_bigint(l bigint, r bigint) returns bool as
$$
begin
    return l is not null and r is null;
end;
$$ language plpgsql;


-- правое не null и оно действительно изменилось относительно левого (в т.ч. и если левое было null)
create or replace function right_neq_bigint(l bigint, r bigint) returns bool as
$$
begin
    return (l is null and r is not null) or (l is not null and r is not null and l != r);
end;
$$ language plpgsql;
