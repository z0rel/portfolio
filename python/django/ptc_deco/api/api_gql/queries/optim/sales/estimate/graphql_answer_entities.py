from django.db import models
from django.db.models import DecimalField

from .......api import models as m
from .......api.models.utils import named_meta, DECIMAL_PRICE_PLACES


DecField = DecimalField(max_digits=20, decimal_places=2)
P = {'max_digits': 20, 'null': True, 'decimal_places': DECIMAL_PRICE_PLACES}


class EstimateStaticAdditionalOthers(models.Model):
    """Таблица суммарных статических доп. расходов"""

    Meta = named_meta(
        'Суммы по статическим дополнительным расходам (дополнительным работам)',
        'EstimateStaticAdditionalOthers',
        managed=False,
    )

    name = models.CharField(max_length=256, help_text='Наименование дополнительного расхода', null=True)
    price = models.DecimalField(max_digits=20, help_text='Сумма', null=True, decimal_places=DECIMAL_PRICE_PLACES)


class EstimateAdditionalOthers(models.Model):
    """Таблица суммарных доп. расходов РТС"""

    Meta = named_meta('Суммы по дополнительным расходам РТС', 'EstimateAdditionalOthers', managed=False)

    name = models.CharField(max_length=256, help_text='Наименование дополнительного расхода', null=True)
    summa_before_discount = models.DecimalField(help_text='Сумма до скидки', **P)
    discount_value = models.DecimalField(help_text='Сумма скидки', **P)
    summa_after_discount = models.DecimalField(help_text='Сумма после скидки', **P)
    discount_percent = models.DecimalField(help_text='Процент скидки', **P)
    agency_commission_value = models.DecimalField(help_text='Сумма агентской комиссии', **P)


class EstimateItogsNonRtsNoSides(models.Model):
    """Таблица суммарных доп. расходов НОН РТС"""

    class Meta:
        managed = False
        verbose_name = 'Суммы по дополнительным расходам НОН РТС не включая рекламные стороны НОН РТС'

    name = models.CharField(max_length=256, help_text='Наименование дополнительного расхода НОН РТС', null=True)
    sale = models.DecimalField(help_text='Сумма продажи', **P)
    pay = models.DecimalField(help_text='Сумма покупки', **P)
    margin = models.DecimalField(help_text='Маржа', **P)
    agency_commission_value = models.DecimalField(help_text='Сумма агентской комиссии', **P)


class EstimateReservationsNonRts(m.projects.ReservationAbstract):
    """Таблица дополнительных расходов по бронированию конструкций НОН РТС"""

    Meta = named_meta(
        'Таблица дополнительных расходов по бронированию конструкций НОН РТС',
        'EstimateReservationsNonRts',
        managed=False,
    )

    sale = models.DecimalField(help_text='Сумма продажи', **P)
    pay = models.DecimalField(help_text='Сумма покупки', **P)
    margin = models.DecimalField(help_text='Маржа', **P)
    agency_commission_value = models.DecimalField(help_text='Сумма агентской комиссии', **P)

    address_title = models.CharField(max_length=512, null=False, help_text='Адрес', unique=True)
    city_title = models.CharField(max_length=64, help_text='Название', null=True)
    format_title = models.CharField(max_length=128, help_text='Наименование формата', null=True)

    city = models.ForeignKey('City', help_text='Город', null=True, on_delete=models.CASCADE)
    format = models.ForeignKey('Format', help_text='Формат', null=True, on_delete=models.CASCADE)
    address = models.ForeignKey('Addresses', help_text='Адрес', null=True, on_delete=models.CASCADE)

    construction_side = models.ForeignKey('ConstructionSide', on_delete=models.DO_NOTHING, null=True)
    reservation_type = models.ForeignKey('ReservationType', on_delete=models.DO_NOTHING, null=True)
    project = models.ForeignKey('Project', on_delete=models.DO_NOTHING, null=True)
    agency_commission = models.ForeignKey('AgencyCommission', null=True, on_delete=models.DO_NOTHING)
    nonrts_part = models.ForeignKey('EstimateNonRts', null=True, on_delete=models.DO_NOTHING)


class EstimateNoSidesNonRts(m.sales.EstimateNonRtsAbstract):
    """Таблица дополнительных расходов по бронированию конструкций НОН РТС"""

    Meta = named_meta(
        'Таблица дополнительных расходов НОН РТС (не бронирований сторон НОН РТС)',
        'EstimateNoSidesNonRts',
        managed=False,
    )

    name = models.CharField(max_length=256, help_text='Наименование дополнительного расхода НОН РТС', null=True)
    nonrts_part = models.ForeignKey('EstimateNonRts', null=True, on_delete=models.DO_NOTHING)
    sale = models.DecimalField(help_text='Сумма продажи', **P)
    pay = models.DecimalField(help_text='Сумма покупки', **P)
    margin = models.DecimalField(help_text='Маржа', **P)
    agency_commission_calculated = models.DecimalField(help_text='Сумма агентской комиссии', **P)
    city = models.ForeignKey('City', null=True, on_delete=models.SET_NULL)
    agency_commission = models.ForeignKey('AgencyCommission', null=True, on_delete=models.DO_NOTHING)


class EstimateAdditionalCostsRts(m.sales.AdditionalCostsAbstract):
    Meta = named_meta('Дополнительные расходы (РТС)', 'AdditionalCosts', managed=False)

    summa_before_discount = models.DecimalField(help_text='Сумма до скидки', **P)
    discount_value = models.DecimalField(help_text='Сумма скидки', **P)
    agency_commission_value = models.DecimalField(help_text='Сумма агентской комиссии', **P)

    summa_after_discount = models.DecimalField(help_text='Сумма после скидки', **P)
    price_after_discount = models.DecimalField(help_text='Стоимость после скидки', **P)
    discount_percent = models.DecimalField(help_text='Процент скидки', **P)

    agency_commission_percent = models.DecimalField(help_text='Процент агентской комиссии', **P)
    value_without_agency_commission = models.DecimalField(help_text='Сумма за вычетом агентской комиссии', **P)

    city = models.ForeignKey(
        'City',
        help_text='Дополнительные расходы -> Город',
        related_name='_estimate_sales_additional_costs',
        null=True,
        on_delete=models.DO_NOTHING,
    )
    agency_commission = models.ForeignKey(
        'AgencyCommission',
        help_text='Дополнительные расходы -> Агентская комиссия',
        null=True,
        on_delete=models.DO_NOTHING,
        related_name='_estimate_additional_costs',
    )
