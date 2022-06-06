import sys
from django.db import transaction
from django.core.management.base import BaseCommand

from .utils import read_yaml, get_families_tree
from ptc_deco.api.models import StaticAdditionalCosts, City, Format


class Command(BaseCommand):
    help = 'Загрузить данные Статические, конфигурационные доп. расходы в базу данных'

    @transaction.atomic
    def handle(self, *args, **options):
        try:
            data = read_yaml('static_additional_costs.yml')
        except FileNotFoundError as e:
            print(e)
            sys.exit()
        else:
            formats = Format.objects.all()

            for s_city in data['cities']:
                city = City.objects.get(title=s_city['city'])
                for format in formats:
                    for values in data['items']:
                        values['city'] = city
                        values['format'] = format
                        StaticAdditionalCosts.objects.get_or_create(**values)
