import json
import yaml

from django.core.management.base import BaseCommand, CommandError

from .upload_model import upload_model_to_dictionary

from ptc_deco.api import models as m


class Command(BaseCommand):
    help = 'Выгрузить базу проектов в Json'

    def add_arguments(self, parser):
        parser.add_argument(
            '--to',
            help='файл, в который будут сохраняться выгруженные данные',
        )
        parser.add_argument(
            '--envfile',
            help='специфический файл настроек',
        )

    def handle(self, *args, **options):
        values = upload_model_to_dictionary()
        if 'to' in options:
            jsonvalue = values.to_json()
            val = json.loads(jsonvalue)
            print(f'write yaml to {options["to"]}')
            with open(options['to'], 'w') as f:
                yaml.dump(val, f)  #, allow_unicode=True)
            print(f'write json to {options["to"] + ".json"}')
            with open(options['to'] + '.json', 'w') as f:
                f.write(jsonvalue)  # , ensure_ascii=False))

        print(len(values.project_list))
