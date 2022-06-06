import sys

from django.db import transaction
from django.core.management.base import BaseCommand
import iso8601

from .utils import read_yaml
from ptc_deco.api.models import ReservationPackage, Package, Project, Estimate, ReservationType


class Command(BaseCommand):
    help = 'Загрузить данные Договор в базу данных'

    @transaction.atomic
    def handle(self, *args, **options):
        try:
            data = read_yaml('reservation_packages.yml')
        except FileNotFoundError as e:
            print(e)
            sys.exit()
        else:
            for _, values in data.items():
                date_field = ('date_from', 'date_to')
                for field in date_field:
                    values[field] = iso8601.parse_date(values[field])
                values['reservation_type'] = ReservationType.objects.get(title=values['reservation_type'])
                values['project'] = Project.objects.get(code=values['project'])
                # values['estimate'] = Estimate.objects.filter(title=values['estimate']).first()
                values['package'] = Package.objects.get(title=values['package'])
                ReservationPackage.objects.get_or_create(**values)
