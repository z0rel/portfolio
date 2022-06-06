from django.db import transaction
from django.core.management.base import BaseCommand

from .utils import get_families_tree


class Command(BaseCommand):
    help = 'Загрузить данные семейств в базу данных'

    @transaction.atomic
    def handle(self, *args, **options):
        families = get_families_tree()

        from ptc_deco.api import models as m
        for family, underfamilies in families.items():
            m_family = m.FamilyConstruction.objects.create(title=family)
            for underfamily, models in underfamilies.items():
                m_underfamily = m.UnderFamilyConstruction.objects.create(title=underfamily, family=m_family)
                for model, formats in models.items():
                    m_model = m.ModelConstruction.objects.create(title=model, underfamily=m_underfamily)
                    for (format_value, format_code), sides in formats.items():
                        m_format = m.Format.objects.create(title=format_value, code=format_code, model=m_model)
                        for side, (adv_sides, size, side_code) in sides.items():
                            m_side = m.Side.objects.create(title=side, size=size, format=m_format, code=side_code)
                            for (adv_side, adv_size_code) in adv_sides:
                                m.AdvertisingSide.objects.create(title=adv_side, side=m_side, code=adv_size_code)
