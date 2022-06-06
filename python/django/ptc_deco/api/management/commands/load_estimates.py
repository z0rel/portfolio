import sys

from django.db import transaction
from django.core.management.base import BaseCommand

from .utils import read_yaml
from ptc_deco.api.models import Estimate, City, Partner


class Command(BaseCommand):
    help = 'Загрузить данные Смета в базу данных'

    @transaction.atomic
    def handle(self, *args, **options):
        try:
            data = read_yaml('estimates.yml')
        except FileNotFoundError as e:
            print(e)
            sys.exit()
        else:
            for _, values in data.items():
                values['city'] = City.objects.get(title=values['city'])
                values['partner'] = Partner.objects.filter(title=values['partner']).first()
                Estimate.objects.get_or_create(**values)
