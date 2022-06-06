-- Function: get_type_object_by_id(bigint, role_group)

DROP FUNCTION get_type_object_by_id(bigint, role_group);

CREATE OR REPLACE FUNCTION get_type_object_by_id(objid bigint, obj_role_group role_group)
  RETURNS character varying AS
$$
declare
  subject_roles_ varchar[];
  local_name_ varchar;
begin

  if (objid >= 0 and objid < 4294967296) -- special logic for certificates
  then
    select array(select R.string_id from roles R inner join user_roles_entries URE on R.role_id = URE.role_id
    where URE.user_cert_id = objid)
    into subject_roles_;

    if (obj_role_group = 'admin')
    then
			return 'admin_certificate';
		elseif (obj_role_group = 'user')
    then
      if (subject_roles_ && array['bu']::varchar[])
      then
        return 'blocked_user_certificate';
      elseif (subject_roles_ && array['u']::varchar[])
      then
        return 'user_certificate';
      end if;
    elseif (obj_role_group is null)
    then
      if (subject_roles_ && array['seca', 'a', 'aud']::varchar[])
      then
        return 'admin_certificate';
      elseif (subject_roles_ && array['bu']::varchar[])
      then
        return 'blocked_user_certificate';
      elseif (subject_roles_ && array['u']::varchar[])
      then
        return 'user_certificate';
      end if;
    end if;

    return 'certificate';
  elseif (objid > 17179869183 and objid < 21474836480)
  then
    -- у каждого системного объекта свой тип
		select local_name from system_objects where obj_id = objid into local_name_;
  else
	  select local_name from ref_object_types
	  where (objid > scope_from or objid = scope_from) and (objid < scope_to or objid = scope_to)
    into local_name_;
  end if;

  return local_name_;
end;
$$
language plpgsql;