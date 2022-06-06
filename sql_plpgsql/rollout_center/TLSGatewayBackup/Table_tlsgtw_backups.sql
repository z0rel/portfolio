drop table tlsgtw_backups;

create table tlsgtw_backups(
  id bigint NOT NULL DEFAULT nextval('backup_id_seq'::regclass),
  dir varchar,
  fname varchar,
  when_created timestamptz,
  commentary varchar,
  file_size bigint,
  when_registered timestamptz default now(),
  id_who_registered bigint,
  constraint backup_files_id_pkey primary key(id),
  CONSTRAINT uq_backup_files_fname_key UNIQUE(fname)
);