# -*- coding: UTF-8 -*-

import os
from fabric import task
from patchwork import files
from invoke.context import Context
import dotenv
from ci.psql import _drop_postgresql_db, _configure_postgresql, _local_configure_postgresql, _local_drop_postgresql_db

from ci.load_data_to_db import _local_copy_proto_front, _load_data_to_db_int

# TODO: При деплое и сносе static - копировать статик фронтэнда в статик бекэнда

from ci.fabtasks import (
    DeployConnection,
    DeployContext,
    _build_whl,
    _deploy_whl_package,
    _distclean,
    _remove_whl_package,
    _deploy_migrate,
    _deploy_migrate_sudo,
    _deploy_nginx_config,
    _deploy_static,
    sudo,
    run_on_cd,
    _deploy_directory
)

from ci.generate_systemd import _deploy_systemd_service
from ci.deploy_dotenv import _deploy_dotenv
from ci.load_data_to_db import _load_data_to_db, _load_data_to_db_clean, _deploy_datasources, _set_maximum_numbers


optargs = {'optional': ['dotenv']}
dotenv_default = '.env.local'


def _setenv(v):
    dotenv.load_dotenv(v)


@task(**optargs)
def build(c, dotenv=dotenv_default):
    """
    Собрать whl пакет
    @type c: Context
    """
    _setenv(dotenv)
    _build_whl()


@task(**optargs)
def distclean(c, dotenv=dotenv_default):
    """
    Очистить локальные артефакты сборки
    @type c: Context
    """
    _setenv(dotenv)
    _distclean()


@task(**optargs)
def clean_deploy(c, dotenv=dotenv_default):
    """
    Очистить все артефакты развертывания на удаленной машине
    @type c: Context
    """
    with DeployConnection(c, dotenv) as (cn, ctx):
        if files.exists(cn, ctx.service_fullpath):
            try:
                cn.sudo(f'systemctl stop {ctx.service_name}')
                cn.sudo(f'systemctl disable {ctx.service_name}')
            except Exception as e:
                print(f'ERROR: {str(e)}')

        if files.exists(cn, ctx.deploy_dir):
            cn.sudo(f'rm -rf {ctx.deploy_dir}')
            print(f'home directory "{ctx.deploy_dir}" removed')

        if not ctx.user_not_exists(cn, ctx.ptcuser):
            cn.sudo(f'userdel {ctx.ptcuser}')
            print(f'backend user "{ctx.ptcuser}" removed')

        if files.exists(cn, ctx.service_fullpath):
            cn.sudo(f'rm {ctx.service_fullpath}')
            cn.sudo(f'systemctl daemon-reload')


@task(**optargs)
def deploy_datasources(c, dotenv=dotenv_default):
    """
    Скопировать на удаленную машину тестовые выгрузки excel
    @type c: Context
    """
    with DeployConnection(c, dotenv) as (cn, ctx):
        _deploy_datasources(cn, ctx)


@task(**optargs)
def deploy_raw_ddl(c, dotenv=dotenv_default):
    """
    Выполнить миграцию хранимых процедур
    @type c: Context
    """
    redeploy_whl(c, dotenv)
    with DeployConnection(c, dotenv) as (cn, ctx):
        _load_data_to_db_int(cn, ctx, fixtures=['create_raw_ddl'], names_to_deploy=[])


@task(**optargs)
def deploy_testrun(c, dotenv=dotenv_default):
    """
    Выполнить тестовую команду запуска бэкенда на удаленной машине
    @type c: Context
    """
    with DeployConnection(c, dotenv) as (cn, ctx):
        ctx.run_on_cd(cn, ctx.deploy_dir, ctx.ptcuser, f'{ctx.service_exec_command}')


@task(**optargs)
def deploy_with_cleanup(c, dotenv=dotenv_default):
    """
    Развернуть бекэнд на удаленной машине с предварительным удалением удаленных артефактов предыдущего развертывания
    @type c: Context
    """
    _setenv(dotenv)
    clean_deploy(c)
    deploy(c)


@task(**optargs)
def deploy(c, dotenv=dotenv_default):
    """
    Развернуть бекэнд на удаленной машине
    @type c: Context
    """
    with DeployConnection(c, dotenv) as (cn, ctx):
        home_created = False

        if not files.exists(cn, ctx.deploy_dir):
            sudo(cn, f'mkdir -p {ctx.deploy_dir}')
            print(f'home directory "{ctx.deploy_dir}" created')
            home_created = True

        if ctx.user_not_exists(cn, ctx.ptcuser):
            sudo(cn, f'useradd -s /bin/bash -d {ctx.deploy_dir} {ctx.ptcuser}')
            print(f'backend user "{ctx.deploy_dir}" created')

        if home_created:
            sudo(cn, f'chown -R {ctx.ptcuser}:{ctx.ptcuser} {ctx.deploy_dir}')
            sudo(cn, f'chmod -R u+rw {ctx.deploy_dir}')
            print(f'changed owner of the "{ctx.deploy_dir}" to the "{ctx.ptcuser}"')

        if not files.exists(cn, ctx.venv_fullpath):
            run_on_cd(ctx, cn, ctx.deploy_dir, ctx.ptcuser, f'{ctx.python} -m venv venv --without-pip')
            run_on_cd(ctx, cn, ctx.deploy_dir, ctx.ptcuser, f'pwd')
            run_on_cd(
                ctx,
                cn,
                ctx.deploy_dir,
                ctx.ptcuser,
                f'source venv/bin/activate && curl https://bootstrap.pypa.io/get-pip.py | {ctx.python}',
            )

            print(f'python virtual environment "{ctx.venv_basename}" is created')

        print('== deploy_whl_package ==')
        _deploy_whl_package(cn, ctx)
        return
        print('== deploy_systemd_service ==')
        _deploy_systemd_service(cn, ctx)

    deploy_static_assets(c, dotenv)
    deploy_media(c, dotenv)


@task(**optargs)
def remove_whl(c, dotenv=dotenv_default):
    """
    Удалить whl пакет бекэнда в виртуальном окружении на удаленной машине
    @type c: Context
    """
    with DeployConnection(c, dotenv) as (cn, ctx):
        _remove_whl_package(cn, ctx)


@task(**optargs)
def deploy_whl(c, dotenv=dotenv_default):
    """
    Установить whl пакет в виртуальном окружении на удаленной машине
    @type c: Context
    """
    with DeployConnection(c, dotenv) as (cn, ctx):
        _deploy_dotenv(cn, ctx)
        _deploy_whl_package(cn, ctx)


@task(**optargs)
def deploy_dotenv(c, dotenv=dotenv_default):
    """
    Скопировать dotenv конфигурацию на удаленную машину
    @type c: Context
    """
    with DeployConnection(c, dotenv) as (cn, ctx):
        _deploy_dotenv(cn, ctx)


@task(**optargs)
def redeploy_whl(c, dotenv=dotenv_default):
    """
    Удалить пакет whl на удаленной машине и установить его заново
    @type c: Context
    """
    _setenv(dotenv)
    remove_whl(c)
    deploy_whl(c)
    deploy_systemd_service(c)


@task(**optargs)
def redeploy_whl_and_test(c, dotenv=dotenv_default):
    """
    Удалить пакет whl на удаленной машине, установить его заново и выполнить тестовую команду запуска
    Django-сервера
    @type c: Context
    """
    _setenv(dotenv)
    _build_whl()
    redeploy_whl(c)
    deploy_testrun(c)


@task(**optargs)
def deploy_systemd_service(c, dotenv=dotenv_default):
    """
    Развернуть сервис systemd на удаленной машине
    @type c: Context
    """
    with DeployConnection(c, dotenv) as (cn, ctx):
        _deploy_systemd_service(cn, ctx)


@task(**optargs)
def deploy_psql_config(c, dotenv=dotenv_default):
    """
    Сконфигурировать postgresql на удаленной машине
    @type c: Context
    """
    with DeployConnection(c, dotenv) as (cn, ctx):
        _configure_postgresql(cn, ctx)


@task(**optargs)
def drop_psql_config(c, dotenv=dotenv_default):
    """
    Удалить базу и пользователя в postgresql на удаленной машине
    @type c: Context
    """
    with DeployConnection(c, dotenv) as (cn, ctx):
        _drop_postgresql_db(cn, ctx)


@task(**optargs)
def local_deploy_psql_config(c, dotenv=dotenv_default):
    """
    Сконфигурировать postgresql на локальной машине
    @type c: Context
    """
    _setenv(dotenv)
    _local_configure_postgresql(DeployContext(True, Context()))


@task(**optargs)
def copy_proto_front(c, dotenv=dotenv_default):
    _setenv(dotenv)
    _local_copy_proto_front()


@task(**optargs)
def local_drop_psql_config(c, dotenv=dotenv_default):
    """
    Удалить базу и пользователя в postgresql на удаленной машине
    @type c: Context
    """
    _setenv(dotenv)
    _local_drop_postgresql_db(DeployContext(True, Context()))


@task(**optargs)
def deploy_load_data_to_db(c, dotenv=dotenv_default, fixtures=None):
    """
    Удалить базу и пользователя в postgresql на удаленной машине
    @type c: Context
    """
    with DeployConnection(c, dotenv) as (cn, ctx):
        _load_data_to_db(cn, ctx, fixtures)


@task(**optargs)
def set_maximum_numbers(c, dotenv=dotenv_default):
    """
    Установить максимальные номера в таблицах
    @type c: Context
    """
    with DeployConnection(c, dotenv) as (cn, ctx):
        _set_maximum_numbers(cn, ctx)


@task(**optargs)
def deploy_load_data_to_db_clean(c, dotenv=dotenv_default, fixtures=None):
    """
    Удалить базу и пользователя в postgresql на удаленной машине
    @type c: Context
    """
    with DeployConnection(c, dotenv) as (cn, ctx):
        _load_data_to_db_clean(cn, ctx, fixtures)



@task(**optargs)
def deploy_migrate(c, dotenv=dotenv_default):
    """
    Применить миграции на удаленной машине
    @type c: Context
    """
    with DeployConnection(c, dotenv) as (cn, ctx):
        _deploy_migrate(cn, ctx)


@task(**optargs)
def deploy_migrate_sudo(c, dotenv=dotenv_default):
    """
    Применить миграции на удаленной машине
    @type c: Context
    """
    with DeployConnection(c, dotenv) as (cn, ctx):
        _deploy_migrate_sudo(cn, ctx)


@task(**optargs)
def deploy_nginx_conf(c, dotenv=dotenv_default):
    """
    Развернуть конфигурацию nginx на удаленной машине
    @type c: Context
    """
    with DeployConnection(c, dotenv) as (cn, ctx):
        _deploy_nginx_config(cn, ctx)


@task(**optargs)
def deploy_static_assets(c, dotenv=dotenv_default):
    """
    Развернуть каталог static на удаленной машине
    @type c: Context
    """
    c = Context()
    with DeployConnection(c, dotenv) as (cn, ctx):
        _deploy_static(c, cn, ctx)


@task(**optargs)
def deploy_media(c, dotenv=dotenv_default):
    """
    Развернуть каталог media на удаленной машине
    @type c: Context
    """
    c = Context()
    with DeployConnection(c, dotenv) as (cn, ctx):
        _deploy_directory(c, cn, ctx, 'media', 'media', 'media')


@task(**optargs)
def tunnel(c, dotenv=dotenv_default):
    """
    Туннелировать порт PostgreSQL на локальную машину для управления из PgAdmin
    @type c: Context
    """
    _setenv(dotenv)
    c = Context()
    local_port = os.getenv('TUNNEL_REMOTE_PG_PORT')
    remote_port = os.getenv('TUNNEL_REMOTE_PORT_ON_REMOTE_MACHINE')
    remote_host = os.getenv('TUNNEL_REMOTE_HOST')
    ctx = DeployContext(False, c)
    cmd = f'ssh -fNq -L {local_port}:{ctx.psql_host}:{remote_port} {remote_host}'
    c.run(cmd, echo=True)


@task(**optargs)
def proto(c, dotenv=dotenv_default):
    """
    Туннелировать порт PostgreSQL на локальную машину для управления из PgAdmin
    @type c: Context
    """
    _setenv(dotenv)
    c = Context()
    cmd = f'protoc --python_out=. ' + os.path.join('ptc_deco', 'api', 'api_gql', 'queries', 'proto', '*.proto')
    print(cmd)
    c.run(cmd, echo=True)
