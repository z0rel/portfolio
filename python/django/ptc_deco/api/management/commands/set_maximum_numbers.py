from django.db import transaction
from django.core.management.base import BaseCommand
from django.db.models import F, Max
from django.db.models.functions import TruncYear
from psqlextra.query import ConflictAction

from .utils import get_families_tree
from ptc_deco.api import models as m


class Command(BaseCommand):
    help = 'Установить максимальные номера в таблицах'

    @transaction.atomic
    def handle(self, *args, **options):
        max_nums_of_projects_by_years = m.Project.objects.all().annotate(
            year=TruncYear('start_date')
        ).values('year').annotate(
            max_year=Max(F('num_in_year'))
        ).values('year', 'max_year')

        for item in max_nums_of_projects_by_years:
            print(item['year'].year, item['max_year'])
            m.projects.LastNumYearProject.objects.on_conflict(['year'], ConflictAction.UPDATE).insert(
                year=item['year'].year,
                number=item['max_year']
            )
