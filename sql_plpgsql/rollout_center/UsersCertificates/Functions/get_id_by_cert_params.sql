-- function get_id_by_cert_params(varchar, varchar);

drop function get_id_by_cert_params(varchar, varchar);

create or replace function get_id_by_cert_params(sn_cert varchar, issuer_name varchar)
  returns bigint
as
$$
declare
  cert_id_ bigint;
begin
  select cert_id from users_certificates where lower(serial_number) = lower(sn_cert) and lower(issuer_dn) = lower(issuer_name) into cert_id_;
  return cert_id_;
end;
$$
language plpgsql;