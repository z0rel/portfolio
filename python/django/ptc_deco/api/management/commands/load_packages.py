import random
import sys

from django.db import transaction
from django.core.management.base import BaseCommand

from .utils import read_yaml
from ptc_deco.api.models import Package, PACKAGE_MODEL_CONFIG


class Command(BaseCommand):
    help = 'Загрузить данные пакеты в базу данных'

    @transaction.atomic
    def handle(self, *args, **options):
        try:
            data = read_yaml('packages.yml')
        except FileNotFoundError as e:
            print(e)
            sys.exit()
        else:
            for field, values in data['packages'].items():
                for title in values:
                    random_year = random.randint(
                        PACKAGE_MODEL_CONFIG['min_valid_year'],
                        PACKAGE_MODEL_CONFIG['max_valid_year']
                    )
                    random_month = random.choice(Package.Month.values)
                    Package.objects.get_or_create(title=title, year=random_year, month=random_month)
