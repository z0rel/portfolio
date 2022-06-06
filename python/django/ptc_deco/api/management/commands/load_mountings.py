import sys

import itertools
from django.db import transaction
from django.core.management.base import BaseCommand
from datetime import timedelta
import datetime

from django.db.models import F, Min
from django.utils.timezone import make_aware

from .utils import read_yaml
import ptc_deco.api.models as m


def get_id_maps(model, key, data):
    real_ids = sorted(set([x.id for x in model.objects.only('id')]))
    real_ids = {i: x for i, x in enumerate(real_ids)}
    yml_ids = [x for x in [values.get(key, None) for values in data] if x is not None]
    yml_ids = {x: i for i, x in enumerate(sorted(set(yml_ids)))}
    return real_ids, yml_ids


class Command(BaseCommand):
    help = 'Загрузить данные Монтажи в базу данных'

    @transaction.atomic
    def _cleanup(self):
        m.Mounting.objects.all().delete()
        m.MountingPhoto.objects.all().delete()

    @transaction.atomic
    def _load(self, data):
        m.Mounting.objects.all().delete()

        repeat = 500
        objs = []
        # real_reservation_ids, yml_reservation_ids = get_id_maps(m.Reservation, 'reservation_id', data)
        # real_design_ids, yml_design_ids = get_id_maps(m.Design, 'design_id', data)
        # real_udesign_ids, yml_udesign_ids = get_id_maps(m.Design, 'unmounting_design_id', data)

        for values in data:
            if 'construction_id' in values:
                construction_id = values.pop('construction_id')
                construction = m.Construction.objects.get(id=construction_id)
                values['construction'] = construction
            if 'construction_side_id' in values:
                construction_side_id = values.pop('construction_side_id')
                construction_side = m.ConstructionSide.objects.get(id=construction_side_id)
                values['construction_side'] = construction_side
            if 'crew_id' in values:
                crew_id = values.pop('crew_id')
                crew = m.Crew.objects.get(id=crew_id)
                values['crew'] = crew
            if 'mounting_task_title' in values:
                mounting_task_title = values.pop('mounting_task_title')
                mt = m.MountingTask.objects.get_or_create(title=mounting_task_title)[0]
                values['mounting_task'] = mt
            # if 'reservation_id' in values:
            #     values['reservation_id'] = real_reservation_ids[yml_reservation_ids[values['reservation_id']]]
            # if 'design_id' in values:
            #     values['design_id'] = real_design_ids[yml_design_ids[values['design_id']]]
            # if 'unmounting_design_id' in values:
            #     values['unmounting_design_id'] = real_udesign_ids[yml_udesign_ids[values['unmounting_design_id']]]

        start_mounting = make_aware(datetime.datetime(year=2020, month=6, day=7))
        end_mounting = make_aware(datetime.datetime(year=2020, month=7, day=7))
        photos = []
        for i in range(repeat):
            for values in data:
                values['start_mounting'] = start_mounting + timedelta(minutes=i)
                values['end_mounting'] = end_mounting + timedelta(minutes=i)
                photos.append(values.get('photos', None))
                mounting = {k: v for k, v in values.items() if k != 'photos'}
                objs.append(m.Mounting(**mounting))

        ids = m.Mounting.objects.bulk_create(objs)
        photos_objs = [
            [m.MountingPhoto(mounting_id=mounting_id.id, **photo) for photo in photo_list]
            for (mounting_id, photo_list) in zip(ids, photos)
            if photo_list
        ]
        m.MountingPhoto.objects.bulk_create(itertools.chain.from_iterable(photos_objs))

    def handle(self, *args, **options):
        try:
            data = read_yaml('mountings.yml')
        except FileNotFoundError as e:
            print(e)
            sys.exit()

        self._cleanup()
        self._load(data)
