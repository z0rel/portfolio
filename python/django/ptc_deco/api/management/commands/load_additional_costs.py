import sys

from django.db import transaction
from django.core.management.base import BaseCommand
import iso8601

from .utils import read_yaml
from ptc_deco.api.models import AdditionalCosts, City, Project, AgencyCommission


class Command(BaseCommand):
    help = 'Загрузить дополнительные расходы базу данных'

    @transaction.atomic
    def handle(self, *args, **options):
        try:
            data = read_yaml('additional_costs.yml')
        except FileNotFoundError as e:
            print(e)
            sys.exit()
        else:
            projects = Project.objects.all()
            date_field = ('start_period', 'end_period')
            count_estimate = 0
            count_title = 0
            for proj in projects:
                for s_city in data['cities']:
                    city = City.objects.get(title=s_city['city'])
                    for values in data['items']:
                        agency_commission_value = (values.pop('agency_commission_value')
                                                   if 'agency_commission_value' in values else None)
                        agency_commission_percent = (values.pop('agency_commission_percent')
                                                     if 'agency_commission_percent' in values else None)

                        for field in date_field:
                            if isinstance(values[field], str):
                                values[field] = iso8601.parse_date(values[field])
                        new_values = {**values}
                        new_values['city'] = city
                        new_values['title'] = f'{values["title"]}'
                        new_values['project'] = proj
                        if agency_commission_value is not None or agency_commission_percent is not None:
                            ak = AgencyCommission.objects.create(
                                to_rent=True,
                                to_nalog=True,
                                to_print=True,
                                to_mount=True,
                                to_additional=True,
                                to_nonrts=True,
                                value=agency_commission_value,
                                percent=agency_commission_percent
                            )
                            new_values['agency_commission'] = ak
                        AdditionalCosts.objects.get_or_create(**new_values)
                        count_estimate += 1
                        count_title += 1
