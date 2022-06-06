import sys

from django.db import transaction
from django.core.management.base import BaseCommand

from .utils import read_yaml
from ptc_deco.api.models import SelfCompanyInfo


def load_self_company_info():
    try:
        data = read_yaml('self_company_info.yml')
    except FileNotFoundError as e:
        print(e)
        sys.exit()
    else:
        for _, values in data.items():
            SelfCompanyInfo.objects.get_or_create(**values)


class Command(BaseCommand):
    help = 'Загрузить данные организации Исполнителя в базу данных'

    @transaction.atomic
    def handle(self, *args, **options):
        load_self_company_info()
