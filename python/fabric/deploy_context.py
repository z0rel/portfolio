import os
from os.path import join
from fabric import Config
import invoke

from .pkgenv import PROJECT_NAME, VERSION


def env2bool(x):
    return int(x) != 0 if x else False


def env2int(x):
    return int(x) if x else 0


class DeployContext:
    def __init__(self, is_local, local_context):
        self.deploy_server = os.getenv('DEPLOY_SERVER')
        self.deploy_dir = os.getenv('DEPLOY_DIR')
        self.remote_frontend_dir = os.getenv('REMOTE_FRONTEND_DIR')
        self.whl_filename = f'{PROJECT_NAME}-{VERSION}-py3-none-any.whl'
        self.distdir = join('.', 'dist')
        self.whlname = join(self.distdir, self.whl_filename)
        self.ptcuser = os.getenv('BACKEND_USER')
        self.cfg = Config(overrides={'sudo': {'password': os.getenv('SUDO_PASSWORD')}})
        self.connect_kwargs = {'key_filename': os.getenv('KEY_FILENAME')}
        self.python = os.getenv('REMOTE_PYTHON_CMD')
        self.venv_basename = 'venv'
        self.venv_fullpath = f'{self.deploy_dir}/{self.venv_basename}'
        self.venv_relpath = f'./{self.venv_basename}'
        self.workers = os.getenv('GUNICORN_WORKERS')
        self.service_name = f'{PROJECT_NAME}_gunicorn.service'
        self.service_fullpath = f'/etc/systemd/system/{self.service_name}'

        workers = f'--workers {self.workers}'
        bind = f'--bind unix:{self.deploy_dir}/main.sock {PROJECT_NAME}.main.wsgi:application'
        guncorn_timeout = os.getenv('GUNICORN_TIMEOUT')
        self.guncorn_timeout = guncorn_timeout if guncorn_timeout else "30"
        self.service_exec_command = f'{self.venv_fullpath}/bin/gunicorn {workers} {bind} ' \
                                    f'--access-logfile={self.deploy_dir}/access.log ' \
                                    f'--error-logfile={self.deploy_dir}/error.log ' \
                                    f'--timeout {self.guncorn_timeout}'

        self.dotenv_build_fname = join(self.distdir, '.env.local')
        self.dotenv_remote_fname = f'{self.deploy_dir}/.env.local'

        self.psql_script_basename = 'postgresql_deploy.sql'
        self.psql_script_local_fname = join(self.distdir, self.psql_script_basename)
        self.psql_script_remote_fname = f'{self.deploy_dir}/{self.psql_script_basename}'
        self.psql_user = os.getenv('PG_USER')
        self.psql_password = os.getenv('PG_PASSWORD')
        self.psql_host = os.getenv('PG_HOST')
        self.psql_port = os.getenv('DEPLOY_REMOTE_PG_PORT')
        self.psql_dbname = os.getenv('PG_DBNAME')
        self.DISABLE_GQL_AUTH_CONTROL = os.getenv('DISABLE_GQL_AUTH_CONTROL')
        self.postgres_admin_user = 'postgres'
        self.psql_db_timezone = 'UTC'  # TODO: TIMEZONE in Database

        self.use_sqlite_text = os.getenv('USE_SQLITE')
        self.django_debug = env2int(os.getenv('DJANGO_DEBUG'))
        self.show_graphiql = env2int(os.getenv('SHOW_GRAPHIQL'))

        self.deploy_locale = os.getenv('DEPLOY_LOCALE')

        self.psql_local_user = os.getenv('PG_LOCAL_USER')
        self.psql_local_password = os.getenv('PG_LOCAL_PASSWORD')

        self.local_context = local_context
        self.is_local = is_local

        self.remote_manage_py = f'{self.deploy_dir}/manage.py'

    def configure_to_local(self):
        self.psql_script_remote_fname = self.psql_script_local_fname
        # self.dotenv_remote_fname - не менять на .env.local иначе перезапишется локальное окружение
        self.venv_fullpath = self.venv_relpath = join('.', self.venv_basename)
        self.remote_manage_py = join('.', 'manage.py')


    def check_distdir_and_create_if_not_exists(self):
        if not os.path.isdir(self.distdir):
            os.mkdir(self.distdir)

    def user_not_exists(self, cn, uname):
        """

        @type uname: str
        @type cn: Connection
        """
        user_not_exists = False
        try:
            cn.run(f'id -u {uname}')
        except invoke.exceptions.UnexpectedExit as e:
            user_not_exists = e.result.exited == 1
        return user_not_exists

    def run_on_cd(self, cn, rundir, user, command, local_run_by_hand=False, perm='-u'):
        """

        @type command: str
        @type user: str
        @type rundir: str
        @type cn: Connection
        """
        if self.is_local:  # выполнить локально
            if local_run_by_hand:
                print('run on terminal:\n')
                print(command)
                print('\n')
                print('and press Enter')
                input()
            else:
                self.local_context.run(command, echo=True, hide=None)
        else:
            tmp_script = f"{rundir}/execscript.sh"
            create_tmpscript_cmd = f'echo "#!/bin/bash\n{command}" > {tmp_script}'
            # print(create_tmpscript_cmd)
            cn.sudo(create_tmpscript_cmd)
            cn.sudo(f'chmod a+x {tmp_script}')
            exec_cmd = f'''bash -c "cd '{rundir}' && sudo {perm} {user} -H -s '{tmp_script}'"'''
            # print(exec_cmd)
            cn.sudo(exec_cmd)
            cn.sudo(f'rm {tmp_script}')
