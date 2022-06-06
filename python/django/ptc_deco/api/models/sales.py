from django.contrib.postgres.indexes import HashIndex, BTreeIndex
from django.db import models
from django.utils import timezone
from django.core.validators import MaxValueValidator, MinValueValidator

from .utils import named_meta, journal_save_delete, DECIMAL_PRICE_PLACES
from .geolocations import City
from .construction import Format
from .projects import Project
from .contragents import Partner
from .contract import Appendix, Contract
from .agency_comission import AgencyCommission, discount_mixin_factory


class EstimateNonRtsAbstract(models.Model):
    Meta = named_meta('Дополнительный расход НОН РТС', 'EstimateNonRts', abstract=True)

    count = models.BigIntegerField(help_text='Количество (НОН РТС)', null=True)
    title = models.CharField(help_text='Тип (наружная реклама, ТВ, радио, метро, другое)', max_length=256, null=True)

    start_period = models.DateTimeField(help_text='Начало периода', null=True)
    end_period = models.DateTimeField(help_text='Конец периода', null=True)

    incoming_rent = models.DecimalField(
        help_text='Входящая стоимость - Аренда', max_digits=20, null=True, decimal_places=DECIMAL_PRICE_PLACES
    )
    incoming_tax = models.DecimalField(
        help_text='Входящая стоимость - Налог', max_digits=20, null=True, decimal_places=DECIMAL_PRICE_PLACES
    )
    incoming_printing = models.DecimalField(
        help_text='Входящая стоимость - Печать', max_digits=20, null=True, decimal_places=DECIMAL_PRICE_PLACES
    )
    incoming_manufacturing = models.DecimalField(
        help_text='Входящая стоимость - Производство', max_digits=20, null=True, decimal_places=DECIMAL_PRICE_PLACES
    )
    incoming_installation = models.DecimalField(
        help_text='Входящая стоимость - Монтаж', max_digits=20, null=True, decimal_places=DECIMAL_PRICE_PLACES
    )
    incoming_additional = models.DecimalField(
        help_text='Входящая стоимость - Доп. расходы', max_digits=20, null=True, decimal_places=DECIMAL_PRICE_PLACES
    )

    sale_rent = models.DecimalField(
        help_text='Стоимость продажи - Аренда', max_digits=20, null=True, decimal_places=DECIMAL_PRICE_PLACES
    )
    sale_tax = models.DecimalField(
        help_text='Стоимость продажи - Налог', max_digits=20, null=True, decimal_places=DECIMAL_PRICE_PLACES
    )
    sale_printing = models.DecimalField(
        help_text='Стоимость продажи - Печать', max_digits=20, null=True, decimal_places=DECIMAL_PRICE_PLACES
    )
    sale_manufacturing = models.DecimalField(
        help_text='Стоимость продажи - Производство', max_digits=20, null=True, decimal_places=DECIMAL_PRICE_PLACES
    )
    sale_installation = models.DecimalField(
        help_text='Стоимость продажи - Монтаж', max_digits=20, null=True, decimal_places=DECIMAL_PRICE_PLACES
    )
    sale_additional = models.DecimalField(
        help_text='Стоимость продажи - Доп. расходы', max_digits=20, null=True, decimal_places=DECIMAL_PRICE_PLACES
    )


@journal_save_delete
class EstimateNonRts(EstimateNonRtsAbstract):
    Meta = named_meta('Дополнительный расход НОН РТС', 'EstimateNonRts')

    city = models.ForeignKey(
        City,
        help_text='Дополнительные расходы НОН РТС -> Город',
        related_name='additional_costs_nonrts',
        null=True,
        on_delete=models.SET_NULL,
    )
    project = models.ForeignKey(
        Project,
        help_text='Дополнительные расходы НОН РТС -> Проект',
        related_name='additional_costs_nonrts',
        null=True,
        on_delete=models.CASCADE,
    )

    # Доп. расходы НОН РТС переносятся из проекта в приложение
    appendix = models.ManyToManyField(
        Appendix,
        help_text='Дополнительные расходы НОН РТС -> Приложение',
        related_name='additional_costs_nonrts',
        blank=True,
    )

    # Стоимость НОН РТС раздела сметы для НОН РТС конструкций
    construction_side = models.ForeignKey(
        'ConstructionSide',
        help_text='Данные о НОН РТС стоимости -> Сторона конструкции',
        related_name='estimate_non_rts',
        null=True,
        on_delete=models.SET_NULL,
    )

    agency_commission = models.ForeignKey(
        AgencyCommission,
        help_text='Дополнительные расходы НОН РТС -> Агентская комиссия',
        null=True,
        on_delete=models.SET_NULL,
        related_name='estimate_non_rts',
    )


# Микисн добавляет поля "скидка на дополнительный расход",
# "стоимость после скидки на дополнительный расход"
class AdditionalCostsAbstract(discount_mixin_factory('на дополнительный расход')):
    Meta = named_meta('Дополнительные расходы (РТС)', 'AdditionalCosts', abstract=True)

    CAT_NALOG = 0
    CAT_MOUNTING = 1
    CAT_PRINTING = 2
    CAT_RENT = 3
    CAT_ADDITIONAL = 4

    title = models.CharField(max_length=512, help_text='Название', null=True)

    start_period = models.DateTimeField(help_text='Начало периода', null=True)

    end_period = models.DateTimeField(help_text='Окончание периода', null=True)

    count = models.IntegerField(help_text='Количество', null=True)

    price = models.DecimalField(help_text='Цена', max_digits=20, null=True, decimal_places=DECIMAL_PRICE_PLACES)

    category = models.IntegerField(
        default=CAT_ADDITIONAL,
        choices=[
            (CAT_NALOG, 'Налог'),
            (CAT_MOUNTING, 'Монтаж'),
            (CAT_PRINTING, 'Печать'),
            (CAT_RENT, 'Аренда'),
            (CAT_ADDITIONAL, 'Дополнительные расходы'),
        ],
        help_text='Категория дополнительного расхода',
    )


@journal_save_delete
class AdditionalCosts(AdditionalCostsAbstract):
    Meta = named_meta('Дополнительные расходы (РТС)', 'AdditionalCosts')

    project = models.ForeignKey(
        Project,
        help_text='Дополнительные расходы -> Проект',
        related_name='additional_costs',
        null=True,
        on_delete=models.SET_NULL,
    )

    city = models.ForeignKey(
        City,
        help_text='Дополнительные расходы -> Город',
        related_name='sales_additional_costs',
        null=True,
        on_delete=models.SET_NULL,
    )

    agency_commission = models.ForeignKey(
        AgencyCommission,
        help_text='Дополнительные расходы -> Агентская комиссия',
        null=True,
        on_delete=models.SET_NULL,
        related_name='additional_costs',
    )

    # Доп. расходы НОН РТС переносятся из проекта в приложение
    appendix = models.ManyToManyField(
        Appendix,
        help_text='Дополнительные расходы НОН РТС -> Приложение',
        related_name='additional_costs',
        blank=True,
    )

    def save(self, *args, **kwargs):
        if self.title:
            lower_title = self.title.lower()
            if 'налог' in lower_title:
                self.category = self.CAT_NALOG
            elif 'монтаж' in lower_title:
                self.category = self.CAT_MOUNTING
            elif 'печать' in lower_title:
                self.category = self.CAT_PRINTING
            elif 'аренда' in lower_title:
                self.category = self.CAT_RENT
            else:
                self.category = self.CAT_ADDITIONAL

        super().save(*args, **kwargs)


@journal_save_delete
class Invoice(models.Model):
    Meta = named_meta('Счет', 'Invoice')

    sum_without_nds = models.DecimalField(
        max_digits=20, help_text='Сумма без НДС', null=True, decimal_places=DECIMAL_PRICE_PLACES
    )
    whole_sum = models.DecimalField(
        max_digits=20, help_text='Общая сумма', null=True, decimal_places=DECIMAL_PRICE_PLACES
    )
    payment_last_date = models.DateTimeField(help_text='Оплата не позднее', null=True)
    customer_payment_method = models.CharField(max_length=256, help_text='Способ оплаты клиентом', null=True)
    avr = models.BooleanField(default=False, help_text='Выставление АВР', null=False)

    # TODO: этого контрагента можно вывести из договора
    partner = models.ForeignKey(
        Partner,
        help_text='Счета -> Контрагент, на котрого выставляется счет',
        related_name='invoices',
        null=True,
        on_delete=models.SET_NULL,
    )

    project = models.ForeignKey(
        Project,
        help_text='Счета -> Проект',
        related_name='invoices',
        null=True,
        on_delete=models.SET_NULL,
    )
    contract = models.ForeignKey(
        Contract,
        help_text='Счета -> Договор',
        related_name='invoices',
        null=True,
        on_delete=models.SET_NULL,
    )
    appendix = models.ForeignKey(
        Appendix,
        help_text='Счета -> Приложение к договору',
        related_name='invoices',
        null=True,
        on_delete=models.SET_NULL,
    )


PACKAGE_MODEL_CONFIG = {
    'min_valid_year': 1970,
    'max_valid_year': timezone.now().year + 100,
}


@journal_save_delete
class Package(models.Model):
    Meta = named_meta('Пакет', 'Package')

    title = models.CharField(max_length=256, help_text='Наименование пакета', null=True)

    city = models.ForeignKey(
        City, help_text='Пакет -> Город', related_name='packages', null=True, on_delete=models.SET_NULL
    )

    year = models.IntegerField(
        help_text="Год",
        default=timezone.now().year,
        validators=[
            MinValueValidator(PACKAGE_MODEL_CONFIG['min_valid_year']),
            MaxValueValidator(PACKAGE_MODEL_CONFIG['max_valid_year']),
        ]
    )

    class Month(models.TextChoices):
        JAN = 'january'
        FEB = 'february'
        MAR = 'march'
        APR = 'april'
        MAY = 'may'
        JUN = 'june'
        JUL = 'july'
        AUG = 'august'
        SEP = 'september'
        OCT = 'october'
        NOV = 'november'
        DEC = 'december'

    month = models.CharField(default=Month.JAN.value, help_text='Месяц', choices=Month.choices, max_length=9)


@journal_save_delete
class PlacementPrice(models.Model):
    Meta = named_meta(
        'Стоимость рекламного размещения по прайсу',
        'PlacementPrice',
        verbose_name_plural='Стоимости рекламного размещения по прайсу',
        unique_together=['city', 'format', 'period'],
    )

    period = models.IntegerField(help_text='Период', null=True)
    price_for_placement = models.DecimalField(
        max_digits=20, help_text='Цена за размещение', null=True, decimal_places=DECIMAL_PRICE_PLACES
    )

    city = models.ForeignKey(
        City,
        help_text='Прайсовая стоимость -> Город',
        related_name='placement_prices',
        null=True,
        on_delete=models.SET_NULL,
    )

    format = models.ForeignKey(
        Format,
        help_text='Прайсовая стоимость -> Формат',
        related_name='placement_prices',
        null=True,
        on_delete=models.SET_NULL,
    )


@journal_save_delete
class StaticAdditionalCosts(models.Model):
    Meta = named_meta(
        'Статические, конфигурационные доп. расходы',
        'StaticAdditionalCosts',
        unique_together=['city', 'format', 'name', 'category'],
        indexes=(BTreeIndex(fields=('category',)), BTreeIndex(fields=('city', 'format', 'category'))),
    )

    CAT_MOUNTING = 0
    CAT_PRINTING = 1
    CAT_NALOG = 2
    CAT_DISCOUNT_NALOG = 3
    CAT_SKETCHES = 4
    CAT_BRANDING = 5
    CAT_ADDITIONAL_MOUNTING = 6
    CAT_ADDITIONAL_PHOTOSET = 7
    CAT_ADDITIONAL_PRINTING = 8
    CAT_ADDITIONAL_DEFAULT = 9

    name = models.CharField(max_length=256, help_text='Наименование дополнительного расхода', null=True)
    price = models.DecimalField(max_digits=20, help_text='Цена', null=True, decimal_places=DECIMAL_PRICE_PLACES)

    city = models.ForeignKey(
        City,
        help_text='Статические, конфигурационные доп. расходы -> Город',
        related_name='static_additional_costs',
        null=True,
        on_delete=models.SET_NULL,
    )

    format = models.ForeignKey(
        Format,
        help_text='Статические, конфигурационные доп. расходы -> Формат',
        related_name='static_additional_costs',
        null=True,
        on_delete=models.SET_NULL,
    )

    category = models.IntegerField(
        default=CAT_ADDITIONAL_DEFAULT,
        choices=[
            (CAT_MOUNTING, 'Монтаж'),
            (CAT_PRINTING, 'Печать'),
            (CAT_NALOG, 'Налог'),
            (CAT_DISCOUNT_NALOG, 'Скидка на налог'),
            (CAT_SKETCHES, 'Согласование эскизов'),
            (CAT_BRANDING, 'Оформление брендированных конструкций'),
            (CAT_ADDITIONAL_MOUNTING, 'Дополнительный монтаж'),
            (CAT_ADDITIONAL_PHOTOSET, 'Дополнительный фотоотчет'),
            (CAT_ADDITIONAL_PRINTING, 'Дополнительная печать'),
        ],
    )

    def save(self, *args, **kwargs):
        if self.name is None:
            if self.category == self.CAT_MOUNTING:
                self.name = 'Монтаж'
            elif self.category == self.CAT_PRINTING:
                self.name = 'Печать'
            elif self.category == self.CAT_NALOG:
                self.name = 'Налог'
            elif self.category == self.CAT_DISCOUNT_NALOG:
                self.name = 'Скидка на налог'
            elif self.category == self.CAT_SKETCHES:
                self.name = 'Согласование эскизов'
            elif self.category == self.CAT_BRANDING:
                self.name = 'Оформление брендированных конструкций'
            elif self.category == self.CAT_ADDITIONAL_MOUNTING:
                self.name = 'Дополнительный монтаж'
            elif self.category == self.CAT_ADDITIONAL_PHOTOSET:
                self.name = 'Дополнительный фотоотчет'
            elif self.category == self.CAT_ADDITIONAL_PRINTING:
                self.name = 'Дополнительная печать'

        super().save(*args, **kwargs)
