import sys

from django.db import transaction
from django.core.management.base import BaseCommand
from django.utils.timezone import make_aware

from .utils import read_yaml
from ptc_deco.api.models import Contract, ContractType, Partner, CustomUser


class Command(BaseCommand):
    help = 'Загрузить данные Договор в базу данных'

    @transaction.atomic
    def handle(self, *args, **options):
        try:
            data = read_yaml('contracts.yml')
        except FileNotFoundError as e:
            print(e)
            sys.exit()
        else:
            for _, values in data.items():
                date_field = ('registration_date', 'start', 'end', 'payment_date')
                for field in date_field:
                    values[field] = make_aware(values[field])
                values['contract_type'] = ContractType.objects.get(name=values['contract_type'])

                p = Partner.objects.filter(title=values['partner'])
                if p:
                    p = p[0]
                    values['partner'] = Partner.objects.get(title=values['partner'])
                    values['creator'] = CustomUser.objects.get(username=values['creator'])
                    values['initiator'] = CustomUser.objects.get(username=values['initiator'])
                    values['sales_manager'] = CustomUser.objects.get(username=values['sales_manager'])
                    Contract.objects.get_or_create(**values)
