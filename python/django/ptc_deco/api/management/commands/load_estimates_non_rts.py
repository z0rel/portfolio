import sys

from django.db import transaction
from django.core.management.base import BaseCommand

from .utils import read_yaml
from ptc_deco.api.models import EstimateNonRts, Project


class Command(BaseCommand):
    help = 'Загрузить данные Смета, раздел НОН РТС в базу данных'

    @transaction.atomic
    def handle(self, *args, **options):
        try:
            data = read_yaml('estimates_non_rts.yml')
        except FileNotFoundError as e:
            print(e)
            sys.exit()
        else:
            for _, values in data.items():

                for i, p in enumerate(Project.objects.all()):
                    if i % 2 == 1:
                        continue
                    values['project'] = p
                    EstimateNonRts.objects.get_or_create(**values)
