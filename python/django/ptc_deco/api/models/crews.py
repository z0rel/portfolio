from django.db import models

from ...logger.mixins import ChangeLoggableMixin
from .utils import named_meta, journal_save_delete
from .geolocations import City


@journal_save_delete
class Crew(models.Model):
    Meta = named_meta('Экипаж', 'Crew')

    num = models.CharField(max_length=64, help_text='Номер экипажа', null=True)
    name = models.CharField(max_length=256, help_text='Имя человека', null=True)
    phone = models.CharField(max_length=64, help_text='Номер телефона монтажника', null=True)

    city = models.ForeignKey(
        City,
        help_text='Экипажи -> Город',
        related_name='crews',
        null=True,
        on_delete=models.SET_NULL
    )

