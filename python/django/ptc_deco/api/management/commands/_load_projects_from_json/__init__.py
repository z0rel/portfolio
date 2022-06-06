import json
import yaml

from django.core.management.base import BaseCommand, CommandError
from django.db import transaction

from .load_model_from_json import load_model_from_json
from .._upload_projects_to_json.entities import UProjectList


class Command(BaseCommand):
    help = 'Загрузить базу проектов из Json'

    def add_arguments(self, parser):
        parser.add_argument(
            '--from',
            help='файл, из которого нужно загрузить базу проектов',
        )
        parser.add_argument(
            '--envfile',
            help='специфический файл настроек',
        )

    @transaction.atomic
    def handle(self, *args, **options):
        if 'from' in options:
            with open(options['from'], 'r') as f:
                obj = UProjectList.from_json(f.read())
            load_model_from_json(obj)
            print(len(obj.project_list))

