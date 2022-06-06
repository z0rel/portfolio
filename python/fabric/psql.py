from fabric import Connection
import psycopg2
from .deploy_context import DeployContext


def execute_local_psql_commands(ctx, commands):
    """

    @type ctx: DeployContext
    """
    connarg = {
        'dbname': 'postgres',
        'user': ctx.psql_local_user,
        'password': ctx.psql_local_password,
        'host': 'localhost'
    }
    with psycopg2.connect(**connarg) as conn:
        conn.set_session(autocommit=True)
        with conn.cursor() as cursor:
            for cmd in commands:
                print(cmd)
                cursor.execute(cmd)


def _drop_commands(ctx):
    commands = [
        f'drop database if exists {ctx.psql_dbname};',
        f'drop role if exists {ctx.psql_user};'
    ]
    done = f'postgresql user "{ctx.psql_user}" and database {ctx.psql_dbname} are dropped successfully'
    return commands, done


def _local_drop_postgresql_db(ctx: DeployContext):
    commands, done = _drop_commands(ctx)
    execute_local_psql_commands(ctx, commands)
    print(f'local {done}')


def _drop_postgresql_db(cn: Connection, ctx: DeployContext):
    commands, done = _drop_commands(ctx)

    ctx.check_distdir_and_create_if_not_exists()
    cn.run(f'systemctl stop {ctx.service_name}')
    with open(ctx.psql_script_local_fname + '.drop', 'w', newline='\n') as f:
        f.write("\n".join(commands + ['']))
    cn.put(ctx.psql_script_local_fname + '.drop', ctx.psql_script_remote_fname + '.drop')

    args = ""
    ctx.run_on_cd(cn, ctx.deploy_dir, 'postgres',
                  f'psql --file={ctx.psql_script_remote_fname}.drop {args} -U postgres', local_run_by_hand=True)

    cn.run(f'rm {ctx.psql_script_remote_fname}.drop')
    print(done)


def _create_commands(ctx):
    commands = [
        f'create user {ctx.psql_user} with password \'{ctx.psql_password}\';',
        f'alter role {ctx.psql_user} set client_encoding to \'utf8\';',
        f'alter role {ctx.psql_user} set default_transaction_isolation to \'read committed\';',
        f'alter role {ctx.psql_user} set timezone to \'{ctx.psql_db_timezone}\';',
        f'create database {ctx.psql_dbname} owner {ctx.psql_user};',
    ]
    done = f'postgresql user "{ctx.psql_user}" and database "{ctx.psql_dbname}" are created successfully'
    return commands, done


def _local_configure_postgresql(ctx: DeployContext):
    commands, done = _create_commands(ctx)
    execute_local_psql_commands(ctx, commands)
    print(f'local {done}')


def _configure_postgresql(cn, ctx):
    """

    @type ctx: DeployContext
    @type cn: Connection
    """
    commands, done = _create_commands(ctx)
    ctx.check_distdir_and_create_if_not_exists()
    with open(ctx.psql_script_local_fname, 'w', newline='\n') as f:
        f.write("\n".join(commands + ['']))
    cn.put(ctx.psql_script_local_fname, ctx.psql_script_remote_fname)

    args = ""
    if ctx.psql_local_user:
        args = f'-U {ctx.psql_local_user}'

    ctx.run_on_cd(cn, ctx.deploy_dir, 'postgres',
                  f'psql --file={ctx.psql_script_remote_fname} {args} -U postgres', local_run_by_hand=True)
    cn.run(f'rm {ctx.psql_script_remote_fname}')
    print(done)



