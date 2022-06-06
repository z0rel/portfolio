import sys
from django.db import transaction
from django.core.management.base import BaseCommand

from ptc_deco.api.models import Partner, WorkingSector
from itertools import cycle


class Command(BaseCommand):
    help = 'Создать сектор деятельности для контрагента'

    @transaction.atomic
    def handle(self, *args, **options):
        working_sectors = WorkingSector.objects.all()
        it_working_sectors = cycle(working_sectors)

        partners = Partner.objects.all()
        for partner in partners:
            partner.working_sectors.add(next(it_working_sectors))
            partner.working_sectors.add(next(it_working_sectors))


