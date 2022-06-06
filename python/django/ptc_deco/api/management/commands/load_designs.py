import sys

from django.db import transaction
from django.core.management.base import BaseCommand

from .utils import read_yaml
from ptc_deco.api.models import Design


class Command(BaseCommand):
    help = 'Загрузить данные Дизайн в базу данных'

    @transaction.atomic
    def handle(self, *args, **options):
        try:
            data = read_yaml('designs.yml')
        except FileNotFoundError as e:
            print(e)
            sys.exit()
        else:
            for _, values in data.items():
                Design.objects.create(**values)
