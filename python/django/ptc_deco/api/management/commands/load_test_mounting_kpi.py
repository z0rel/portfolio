import datetime
import random
from django.db import transaction
from django.core.management.base import BaseCommand
from django.utils import timezone
from django.utils.timezone import make_aware

from ptc_deco.api.models import Crew, Country, City, Mounting


class Command(BaseCommand):
    help = 'Создать монтирования'

    @transaction.atomic
    def handle(self, *args, **options):
        country = Country.objects.create(
            title='TestCountry3'
        )
        city = City.objects.create(
            title='TestCity3',
            country=country
        )
        crew = Crew.objects.create(
            num='test_num', name='Test Name', phone='test_phone', city=city
        )
        print('crew id', crew.id)

        today = timezone.now()

        for i in range(100):
            random_datetime = datetime.datetime(
                year=today.year,
                month=random.randint(1, 12),
                day=random.randint(1, 28),
                hour=random.randint(0, 23)
            )

            obj = Mounting.objects.create(
                start_mounting=make_aware(random_datetime),
                mounting_done=True,
                unmounting_done=False,
                archived=False,
                crew=crew
            )
