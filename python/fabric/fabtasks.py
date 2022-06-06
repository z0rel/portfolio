# -*- coding: UTF-8 -*-
import os
from os.path import join
from fabric import Connection
import invoke
from patchwork import files
import shutil
import dotenv
from typing import Tuple

from .pkgenv import PROJECT_NAME
from .deploy_context import DeployContext, env2bool


def sudo(cn, cmd):
    print('run: ' + cmd)
    cn.sudo(cmd)


def run_on_cd(ctx, cn, deploy_dir, user, cmd):
    print(f'directory: {deploy_dir}, user: {user}, command: {cmd}')
    ctx.run_on_cd(cn, deploy_dir, user, cmd)


class DeployConnection(object):
    """ Контекст соединения для работы с командной строкой """

    def __init__(self, context: DeployContext, dotenv_arg: Connection):
        dotenv.load_dotenv(dotenv_arg)
        self.c = context
        self.local = env2bool(os.getenv('DEPLOY_DBMS_LOCAL'))
        ctx = DeployContext(self.local, context)
        if self.local:
            ctx.configure_to_local()

        self.ctx = ctx
        self.cn = None

    def __enter__(self) -> Tuple[Connection, DeployContext]:
        if not self.local:
            ctx = self.ctx
            self.cn = Connection(ctx.deploy_server, config=ctx.cfg, connect_kwargs=ctx.connect_kwargs)
            self.cn.__enter__()
        else:
            self.cn = self.c
            self.c.sudo = self.c.run
            self.c.put = self.put

        return self.cn, self.ctx

    def __exit__(self, exception_type, value, traceback):
        if not self.local:
            self.cn.__exit__(exception_type, value, traceback)

    def put(self, local, remote):
        pass


class WhlPutter(object):
    """ Контекст для закачки и удаления whl-файла """

    def __init__(self, cn, whlname, whl):
        """

        @type cn: Connection
        @type whl: str
        @type whlname: str
        """
        self.cn = cn
        self.whl = whl
        self.whlname = whlname

    def __enter__(self):
        if files.exists(self.cn, self.whl):
            rm_whl = f'rm {self.whl}'
            print(rm_whl)
            self.cn.run(rm_whl)

        print(f'put whl {self.whlname} to remote')
        self.cn.put(self.whlname, self.whl)

        return self.whl

    def __exit__(self, exception_type, value, traceback):
        rm_whl = f'rm {self.whl}'
        print(rm_whl)
        self.cn.run(rm_whl)


def _build_whl():
    print('== build whl ==')
    lc = invoke.context.Context()
    cmd = 'python setup.py bdist_wheel'
    print(cmd)
    lc.run(cmd)

    try:
        cmd = f'{PROJECT_NAME}.egg-info'
        print(f'rmtree {cmd}')
        shutil.rmtree(cmd)
    except FileNotFoundError:
        pass


def _remove_whl_package(cn, ctx):
    """

    @type ctx: DeployContext
    @type cn: Connection
    """
    if files.exists(cn, ctx.venv_fullpath):
        cmd = f'source ./{ctx.venv_basename}/bin/activate && {ctx.python} -m pip uninstall -y {PROJECT_NAME}'
        run_on_cd(ctx, cn, ctx.deploy_dir, ctx.ptcuser, cmd)
        print(f'backend package uninstall successfully')


def _deploy_whl_package(cn, ctx):
    """

    @type ctx: DeployContext
    @type cn: Connection
    """
    if files.exists(cn, ctx.venv_fullpath):
        _distclean()
        _build_whl()
        with WhlPutter(cn, ctx.whlname, f'{ctx.deploy_dir}/{ctx.whl_filename}') as whl:
            cmd_install_whl = f'source ./{ctx.venv_basename}/bin/activate && {ctx.python} -m pip install {whl}'
            print(f'EXECUTE INSTALL COMMAND: user: {ctx.ptcuser}, dir: {ctx.deploy_dir}, cmd: {cmd_install_whl}')
            run_on_cd(ctx, cn, ctx.deploy_dir, ctx.ptcuser, cmd_install_whl)

        manage_py_put = join('.', 'manage.py')
        print(f'cn.put({manage_py_put}, {ctx.remote_manage_py})')
        cn.put(join('.', 'manage.py'), ctx.remote_manage_py)

        chmod_cmd = f'chmod ug+rx {ctx.remote_manage_py}'
        print(chmod_cmd)
        cn.run(chmod_cmd)

        print(f'backend package installed successfully')


def _deploy_migrate(cn, ctx):
    """

    @type ctx: DeployContext
    @type cn: Connection
    """
    if ctx.is_local:
        cmd = join('.', 'manage.py') + ' migrate'
    else:
        cmd = f'source ./{ctx.venv_basename}/bin/activate && python3 ./manage.py migrate'

    ctx.run_on_cd(cn, ctx.deploy_dir, ctx.ptcuser, cmd, local_run_by_hand=True)


def _deploy_migrate_sudo(cn, ctx):
    """

    @type ctx: DeployContext
    @type cn: Connection
    """
    if ctx.is_local:
        cmd = join('.', 'manage.py') + ' migrate'
    else:
        cmd = f'source ./{ctx.venv_basename}/bin/activate && python3 ./manage.py migrate'

    ctx.run_on_cd(cn, ctx.deploy_dir, 'su -c', cmd, local_run_by_hand=True, perm='')


def _distclean():
    print('== distclean ==')
    clean_files = ['build', 'dist', f'{PROJECT_NAME}.egg-info']
    for f in clean_files:
        try:
            print(f'rmtree {f}')
            shutil.rmtree(f)
        except FileNotFoundError:
            pass


def _deploy_nginx_config(cn, ctx):
    """

    @type ctx: DeployContext
    @type cn: Connection
    """
    server_name = os.getenv('SERVER_NAME')
    # if you has error with the dhparam.pem - run
    # openssl dhparam -out  /etc/ssl/certs/dhparam.pem 4096
    # or if its too long, then
    # openssl dhparam -dsaparam -out /etc/ssl/certs/dhparam.pem 4096

    cn.put(join('.', 'ci', 'assets', 'nginx.conf'), f'{ctx.deploy_dir}/nginx.conf')
    cn.run(f'sed -i "s/<SERVER_NAME>/{server_name}/g" {ctx.deploy_dir}/nginx.conf')
    cn.sudo(f'mv {ctx.deploy_dir}/nginx.conf /etc/nginx/nginx.conf')
    cn.sudo(f'chown nginx:nginx /etc/nginx/nginx.conf')

    if not files.exists(cn, '/etc/nginx/conf.d'):
        cn.sudo('mkdir /etc/nginx/conf.d')

    for item in ['00ssl.conf', '04letsencrypt.conf', '05django.conf']:
        cn.put(join('.', 'ci', 'assets', item), f'{ctx.deploy_dir}/{item}')
        cn.run(f'sed -i "s/<SERVER_NAME>/{server_name}/g" {ctx.deploy_dir}/{item}')
        pat_deploy_dir = ctx.deploy_dir.replace('/', '\\/')
        cn.run(f'sed -i "s/<BACKEND_SOCK_DIR>/{pat_deploy_dir}/g" {ctx.deploy_dir}/{item}')
        cn.run(f'sed -i "s/<DEPLOY_DIR>/{pat_deploy_dir}/g" {ctx.deploy_dir}/{item}')

        cn.sudo(f'mv {ctx.deploy_dir}/{item} /etc/nginx/conf.d/{item}')
        cn.sudo(f'chown nginx:nginx /etc/nginx/conf.d/{item}')
        linkname = f'/etc/nginx/sites-enabled/{item}'
        cn.sudo(f'rm -f {linkname}')
        cn.sudo(f'ln -s /etc/nginx/conf.d/{item} {linkname}')

    cn.sudo('nginx -t')
    cn.sudo('systemctl restart nginx')
    cn.sudo('systemctl status nginx')


def _deploy_directory(c, cn, ctx, src_name, dst_name, local_archive_name):
    """

    @type ctx: DeployContext
    @type cn: Connection
    """
    remote_directory = f'{ctx.deploy_dir}/{dst_name}'
    print(f'remote_static: {remote_directory}')

    if not files.exists(cn, remote_directory):
        cmd = f'mkdir {remote_directory}'
        print(f'create remote directory: {cmd}')
        cn.run(cmd)

    ctx.check_distdir_and_create_if_not_exists()

    local_directory = src_name
    local_directory_zip = join(ctx.distdir, f'{local_archive_name}.zip')
    try:
        print(f'try: remove local zip "{local_directory_zip}"')
        os.remove(local_directory_zip)
    except FileNotFoundError:
        pass

    cmd = f'zip -r {local_directory_zip} {local_directory}'
    print(f'local run command: "{cmd}"')
    c.run(cmd)

    def echo_run_remote(cmd):
        print(f'remote run command: {cmd}')
        cn.run(cmd)

    echo_run_remote(f'rm -f /tmp/{dst_name}.zip')

    remote_archive_name = f'/tmp/{dst_name}.zip'
    print(f'copy to remote: from local {local_directory_zip} to remote {remote_archive_name}')
    cn.put(local_directory_zip, remote_archive_name)

    echo_run_remote(f'unzip -o /tmp/{dst_name}.zip -d /tmp')
    echo_run_remote(f'rm /tmp/{dst_name}.zip')
    echo_run_remote(f'chown -R {ctx.ptcuser}:{ctx.ptcuser} /tmp/{PROJECT_NAME}')
    echo_run_remote(f'rm -rf {ctx.deploy_dir}/{dst_name}')
    echo_run_remote(f'mv /tmp/{PROJECT_NAME}/{dst_name} {ctx.deploy_dir}/')
    echo_run_remote(f'rm -rf /tmp/{PROJECT_NAME}')


def _deploy_static(c, cn, ctx):
    """

    @type ctx: DeployContext
    @type cn: Connection
    """
    _deploy_directory(c, cn, ctx, join('.', PROJECT_NAME, 'static'), 'static', 'static')

    copy_frontend_cmd = f'cp -frp {ctx.remote_frontend_dir}/static/* {ctx.deploy_dir}/static/'
    cn.run(copy_frontend_cmd)
