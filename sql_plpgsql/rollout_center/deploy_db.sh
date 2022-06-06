#!/bin/sh

CUR_DIR=$(dirname $0)

mkdir -p /opt/itcs/postgres_data/nginxdbproxy_index
mkdir -p /opt/itcs/postgres_data/nginxdbproxy_data
chown -R postgres:postgres /opt/itcs/postgres_data/nginxdbproxy_data /opt/itcs/postgres_data/nginxdbproxy_index
chmod -R go-rwx /opt/itcs/postgres_data/nginxdbproxy_index /opt/itcs/postgres_data/nginxdbproxy_data

su - postgres -c '/opt/itcs/bin/createdb tlsgtw_database'
/opt/itcs/bin/psql -U postgres -d tlsgtw_database -f $CUR_DIR/NginxDBProxy/Tablespace_nginxdbproxy.sql

# create sequences before
/opt/itcs/bin/psql -U postgres -d tlsgtw_database -f $CUR_DIR/Sequences_objects_id.sql

# create tables
find $CUR_DIR/*/*.sql ! -name "*_entries.sql" ! -name "*_entry.sql" ! -name "*_dependencies.sql" | while read SQL_FILE; do
    /opt/itcs/bin/psql -U postgres -d tlsgtw_database -f $SQL_FILE
done

# create depend tables
find $CUR_DIR/*/*entries.sql $CUR_DIR/*/*entry.sql | while read SQL_FILE; do
    /opt/itcs/bin/psql -U postgres -d tlsgtw_database -f $SQL_FILE
done

# create depend tables
find $CUR_DIR/*/*dependencies.sql | while read SQL_FILE; do
    /opt/itcs/bin/psql -U postgres -d tlsgtw_database -f $SQL_FILE
done

# create functions
find $CUR_DIR/*/Functions/*.sql | while read SQL_FILE; do
    /opt/itcs/bin/psql -U postgres -d tlsgtw_database -f $SQL_FILE
done
