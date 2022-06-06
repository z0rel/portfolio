import sys
from django.db import transaction
from django.core.management.base import BaseCommand

from .utils import read_yaml
from ptc_deco.api.models import PlacementPrice, City, Format


class Command(BaseCommand):
    help = 'Загрузить данные Прайсовая стоимость в базу данных'

    @transaction.atomic
    def handle(self, *args, **options):
        try:
            data = read_yaml('placement_price.yml')
        except FileNotFoundError as e:
            print(e)
            sys.exit()
        else:
            for _, values in data.items():
                city_name = values.pop('city')
                values['city'] = City.objects.get(title=city_name)
                if 'format' in values:
                    format_name = values.pop('format')
                    formats = Format.objects.filter(title=format_name)
                    if not formats:
                        raise Exception(f'Не найден формат "{format_name}", для записи {city_name} {values}')
                    for fmts in formats:
                        values['format'] = fmts
                        if not PlacementPrice.objects.filter(city=values['city'], format=values['format'], period=values['period']):
                            PlacementPrice.objects.get_or_create(**values)
                # elif 'format_id' in values:
                #     values['format'] = Format.objects.get(id=values.pop('format_id'))
                #     PlacementPrice.objects.get_or_create(**values)
