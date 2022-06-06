import sys

from django.db import transaction
from django.core.management.base import BaseCommand
import iso8601

from .utils import read_yaml
from ptc_deco.api.models import Invoice, Partner, Project, Contract, Appendix


class Command(BaseCommand):
    help = 'Загрузить данные Счета в базу данных'

    @transaction.atomic
    def handle(self, *args, **options):
        try:
            data = read_yaml('invoices.yml')
        except FileNotFoundError as e:
            print(e)
            sys.exit()
        else:
            for _, values in data.items():
                values['invoice_payment_date'] = iso8601.parse_date(values['invoice_payment_date'])
                values['partner_who_is_invoices'] = Partner.objects.get(title=values['partner_who_is_invoices'])
                values['project'] = Project.objects.get(code=values['project'])
                values['contract'] = Contract.objects.get(code=values['contract'])
                values['appendix'] = Appendix.objects.get(code=values['appendix'])
                Invoice.objects.get_or_create(**values)
