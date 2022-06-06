from django.db import transaction
from django.core.management.base import BaseCommand
from ptc_deco.api.models import ConstructionSide, Package


class Command(BaseCommand):
    help = 'Загрузить Пакеты для стороны конструкции в базу данных'

    @transaction.atomic
    def handle(self, *args, **options):
        packages = Package.objects.all()
        construction_sides = ConstructionSide.objects.all()
        count = 0
        stop_count = len(packages) - 1
        # По кругу присвоить рандомный i-й пакет очередной конструкции.
        # Если пакеты закончились, начать брать пакеты с начала списка
        for construction_side in construction_sides:
            if count == stop_count:
                count = 0
            construction_side.package = packages[count]
            construction_side.save()
            count += 1
