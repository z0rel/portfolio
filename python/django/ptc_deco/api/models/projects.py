from django.db import models
from django.db.models import DecimalField, Q
from django.db.models.functions import Cast
from decimal import Decimal
from ptc_deco.main import settings
from django.core.mail import send_mail
from psqlextra.models import PostgresModel

from ...logger.middleware import LoggedInUser

from .utils import named_meta, journal_save_delete, IMAGE_URL_MAX_LENGTH, DECIMAL_PRICE_PLACES, \
    get_construction_side_code
from .construction import ConstructionSide
from .users import CustomUser
from .agency_comission import AgencyCommission, discount_mixin_factory, DiscountMixinPartnerAndProject

DecField = DecimalField(max_digits=20, decimal_places=2)
P = {'max_digits': 20, 'null': True, 'decimal_places': DECIMAL_PRICE_PLACES}


@journal_save_delete
class Design(models.Model):
    Meta = named_meta('Дизайн', 'Design')

    img = models.ImageField(upload_to='design', help_text='Изображение', null=True, max_length=IMAGE_URL_MAX_LENGTH)
    is_current = models.BooleanField(default=False, help_text='Текущий дизайн', null=False)
    started_at = models.DateTimeField(help_text='Дата начала действия дизайна', null=True)
    title = models.CharField(max_length=512, help_text='Название дизайна', null=True)
    archived = models.BooleanField(help_text='Дизайн в архиве', null=False, default=False)

    advert_promo_company = models.ForeignKey(
        'AdvertPromoCompany',
        help_text='Дизайн -> Рекламная компания',
        related_name='designs',
        null=True,
        on_delete=models.CASCADE
    )


@journal_save_delete
class Brand(models.Model):
    Meta = named_meta('Бренд', 'Brand')

    code = models.CharField(max_length=128, help_text='Код', null=True)
    title = models.CharField(max_length=512, help_text='Наименование', null=True)

    created_at = models.DateTimeField(auto_now_add=True, help_text='Дата создания', null=True)
    updated_at = models.DateTimeField(auto_now=True, help_text='Дата обновления', null=True)

    designs = models.ManyToManyField(
        Design,
        help_text='Связанные дизайны',
        related_name='brands_for_design',
        blank=True
    )

    # TODO: Разобраться, нужен ли сектор деятельности в бренде
    working_sector = models.ForeignKey(
        'WorkingSector',
        on_delete=models.SET_NULL,
        help_text='Сектор деятельности контрагента',
        related_name='brands',
        null=True
    )


# Добавляются поля "Процент скидки на налог" и "Процент скидки по прайсу"
@journal_save_delete
class Project(DiscountMixinPartnerAndProject):
    Meta = named_meta('Проект', 'Project')

    code = models.CharField(max_length=128, help_text='Код', null=True)
    num_in_year = models.BigIntegerField(help_text='Порядковый номер проекта', null=True)
    title = models.CharField(max_length=512, help_text='Наименование', null=True)

    comment = models.CharField(max_length=512, help_text='Коментарий', null=True)

    start_date = models.DateTimeField(help_text='Дата начала', null=True)
    created_at = models.DateTimeField(auto_now_add=True, help_text='Дата создания', null=True)
    updated_at = models.DateTimeField(auto_now=True, help_text='Дата обновления', null=True)

    brand = models.ForeignKey(
        Brand,
        on_delete=models.SET_NULL,
        related_name='projects',
        help_text='Бренд',
        null=True
    )

    client = models.ForeignKey(
        'Partner',
        on_delete=models.SET_NULL,
        related_name='projects',
        help_text='Клиент',
        null=True
    )

    agency = models.ForeignKey(
        'Partner',
        on_delete=models.SET_NULL,
        related_name='projects_agencies',
        help_text='Рекламное агентство',
        null=True
    )

    is_archive = models.BooleanField(default=False, help_text='В архиве')

    creator = models.ForeignKey(
        CustomUser,
        help_text='Проекты -> Создатель (кто внес данные)',
        related_name='created_projects',
        null=True,
        on_delete=models.SET_NULL
    )

    back_office_manager = models.ForeignKey(
        CustomUser,
        help_text='Проекты -> Менеджер Бэк-оффиса',
        related_name='back_office_managers_of_projects',
        null=True,
        on_delete=models.SET_NULL
    )

    sales_manager = models.ForeignKey(
        CustomUser,
        help_text='Проекты -> Менеджер по продажам',
        related_name='sales_manager_of_projects',
        null=True,
        on_delete=models.SET_NULL
    )

    agency_commission = models.ForeignKey(
        AgencyCommission,
        help_text='Проекты -> Агентская комиссия',
        null=True,
        on_delete=models.SET_NULL,
        related_name='projects',
    )

    def save(self, *args, **kwargs):
        if 'created_at_internal' in kwargs:
            val = kwargs.pop('created_at_internal')
            self.created_at = val

        super().save(*args, **kwargs)

    def __str__(self):
        return f"{self.title} - {self.id}"


class ProjectCities(models.Model):
    Meta = named_meta('Города для проекта', 'ReservationType', unique_together=['project', 'city'])

    project = models.ForeignKey(
        Project,
        help_text='Города для проектов -> проект',
        related_name='project_cities',
        null=False,
        on_delete=models.CASCADE
    )

    city = models.ForeignKey(
        'City',
        help_text='Проекты -> Менеджер по продажам',
        related_name='sales_manager_of_projects',
        null=False,
        on_delete=models.CASCADE
    )

    count = models.IntegerField(help_text='Количество бронирований в заданном городе и проекте', null=False, default=0)
    saled_count = models.IntegerField(help_text='Количество проданных сторон', null=False, default=0)
    distributed_count = models.IntegerField(help_text='Количество сторон, распределенных на монтаж', null=False,
                                            default=0)


class ReservationType(models.Model):
    Meta = named_meta('Вид бронирования', 'ReservationType')

    class ReservationTypeInteger(models.IntegerChoices):
        FREE = 0, 'Free'
        RESERVED = 1, 'Reserved'
        CANCELLED = 2, 'Cancelled'
        SETTED = 3, 'Confirmed'
        SALED = 4, 'Saled'
        UNAVAILABLE = 5, 'Unavailable'

    title = models.CharField(max_length=128, null=True, help_text='Тип брони', unique=True)
    level = models.IntegerField(null=False, help_text='уровень бронирования')
    ikey = models.IntegerField(choices=ReservationTypeInteger.choices, help_text='целочисленный ключ', null=True,
                               unique=True)

    def __str__(self):
        return self.title


class ReservationAbstract(models.Model):
    Meta = named_meta('Бронирование - базовый класс', 'ReservationAbstract', abstract=True)

    date_from = models.DateTimeField(help_text='Дата начала', null=False)
    date_to = models.DateTimeField(help_text='Дата окончания', null=False)

    branding = models.BooleanField(default=False, help_text='брендинг', null=False)
    # design = models.BooleanField(default=False, null=False)  # TODO: help_text


class ReservationAbstract2(ReservationAbstract):
    Meta = named_meta('Бронирование - базовый класс',
                      'ReservationAbstract',
                      db_table='api_reservation',
                      abstract=True)

    # TODO: добавить в вычислительную модель
    rent_by_price_setted = models.DecimalField(help_text=f'Заданное значение аренды по прайсу', max_digits=20,
                                               null=True, decimal_places=DECIMAL_PRICE_PLACES)

    mounting_setted = models.DecimalField(help_text=f'Заданное значение стоимости монтажа', max_digits=20,
                                          null=True, decimal_places=DECIMAL_PRICE_PLACES)
    printing_setted = models.DecimalField(help_text=f'Заданное значение стоимости печати', max_digits=20,
                                          null=True, decimal_places=DECIMAL_PRICE_PLACES)
    additional_setted = models.DecimalField(help_text=f'Заданное значение стоимости доп. расходов', max_digits=20,
                                            null=True, decimal_places=DECIMAL_PRICE_PLACES)
    nalog_setted = models.DecimalField(help_text=f'Заданное значение стоимости налогов', max_digits=20,
                                       null=True, decimal_places=DECIMAL_PRICE_PLACES)

    discount_price_percent_setted = models.DecimalField(help_text=f'Заданный процент скидки по прайсу', max_digits=20,
                                                        null=True, decimal_places=DECIMAL_PRICE_PLACES)

    rent_by_price_after_discount_setted = models.DecimalField(help_text=f'Заданная стоимость после скидки по прайсу',
                                                              max_digits=20, null=True, blank=True,
                                                              decimal_places=DECIMAL_PRICE_PLACES)

    rent_to_client_setted = models.DecimalField(help_text='Заданная аренда на клиента', max_digits=20, null=True,
                                                decimal_places=DECIMAL_PRICE_PLACES)

    discount_to_client_percent_setted = models.DecimalField(help_text=f'Заданный процент скидки Аренды на клиента',
                                                            max_digits=20, null=True,
                                                            decimal_places=DECIMAL_PRICE_PLACES)

    rent_to_client_after_discount_setted = models.DecimalField(help_text=f'Заданная стоимость Аренды после всех скидок',
                                                               max_digits=20, null=True, blank=True,
                                                               decimal_places=DECIMAL_PRICE_PLACES)

    discount_nalog_percent_setted = models.DecimalField(help_text=f'Заданный процент скидки на налог',
                                                        max_digits=20, null=True,
                                                        decimal_places=DECIMAL_PRICE_PLACES)

    nalog_after_discount_setted = models.DecimalField(help_text=f'Заданная стоимость налога после скидки',
                                                      max_digits=20, null=True, blank=True,
                                                      decimal_places=DECIMAL_PRICE_PLACES)


# @journal_save_delete
class Reservation(ReservationAbstract2):
    Meta = named_meta('Бронирование', 'Reservation')

    creation_date = models.DateTimeField(auto_now_add=True, null=False, blank=True, help_text='Дата создания')

    # Стоимость НОН РТС раздела сметы для НОН РТС конструкций
    estimate_non_rts = models.OneToOneField(
        'EstimateNonRts',
        help_text='Данные о НОН РТС стоимости -> Бронирование',
        related_name='reservation',
        null=True,
        on_delete=models.SET_NULL,
        blank=True
    )

    construction_side = models.ForeignKey(
        ConstructionSide,
        help_text='Бронирования -> Сторона конструкции',
        on_delete=models.DO_NOTHING,
        related_name='reservation',
        null=True
    )
    reservation_type = models.ForeignKey(
        ReservationType,
        help_text='Бронирования -> Тип брони',
        on_delete=models.DO_NOTHING,
        related_name='reservation',
        null=False
    )
    project = models.ForeignKey(
        Project,
        on_delete=models.DO_NOTHING,
        help_text='Бронирования -> Проект',
        related_name='reservations',
        null=True
    )
    agency_commission = models.ForeignKey(
        AgencyCommission,
        help_text='Бронирование -> Агентская комиссия',
        null=True,
        on_delete=models.SET_NULL,
        related_name='reservations',
    )
    appendix = models.ManyToManyField(
        'Appendix',
        help_text='Бронирование <-> Приложение к договору',
        related_name='reservations',
        blank=True
    )
    distributed_to_mounting = models.IntegerField(
        help_text='Распределений на монтаж',
        null=False,
        default=0,
    )

    reservation_package = models.ForeignKey(
        'ReservationPackage',
        help_text='Бронирование -> Бронирование пакета',
        null=True,
        related_name='reservations',
        on_delete=models.CASCADE,
    )

    def send_mail_about_canceled(self):
        subject = f'Ваша бронь отменена'
        message = f'Бронь стороны {get_construction_side_code(self.construction_side.construction.postcode.title, self.construction_side.construction.num_in_district, self.construction_side.advertising_side.side.format.code, self.construction_side.advertising_side.side.code, self.construction_side.advertising_side.code)} с {self.date_from.date()} по {self.date_to.date()} была отменена'
        from_email = settings.EMAIL_HOST_USER
        to_emails = [self.project.client.email]
        send_mail(subject, message, from_email, to_emails)

    def save(self, *args, **kwargs):
        if self.date_from is None and self.date_to is None:
            raise Exception("Дата начала и дата окончания должны быть заданы")
        elif self.date_from is None:
            raise Exception("Дата начала должна быть задана")
        elif self.date_to is None:
            raise Exception("Дата окончания должна быть задана")
        if self.discount_price_percent_setted is not None and self.rent_by_price_after_discount_setted is not None:
            raise Exception("Скидка по прайсу не может быть задана как % и значение после скидки одновременно")
        if self.discount_to_client_percent_setted is not None and self.rent_to_client_after_discount_setted is not None:
            raise Exception("Скидка на клиента не может быть задана как % и значение после скидки одновременно")
        if self.discount_nalog_percent_setted is not None and self.nalog_after_discount_setted is not None:
            raise Exception("Скидка на налог не может быть задана как % и значение после скидки одновременно")

        if self.reservation_type.ikey == ReservationType.ReservationTypeInteger.CANCELLED:
            self.send_mail_about_canceled()

        if self.reservation_type.ikey == ReservationType.ReservationTypeInteger.SALED:
            conflict_reservations = Reservation.objects.filter(
                (Q(date_from__lte=self.date_from) & Q(date_to__gte=self.date_from)) |
                (Q(date_from__lte=self.date_to) & Q(date_to__gte=self.date_to)) |
                (Q(date_from__gte=self.date_from) & Q(date_to__lte=self.date_to)),
                construction_side__construction_id=self.construction_side.id,
                reservation_type__ikey=ReservationType.ReservationTypeInteger.RESERVED)
            if self.pk is not None:
                conflict_reservations = conflict_reservations.exclude(pk=self.pk)
            for res in conflict_reservations:
                res.reservation_type = ReservationType.objects.get(
                    ikey=ReservationType.ReservationTypeInteger.CANCELLED)
                res.save()

        super().save(*args, **kwargs)


def _calc_discount_percent(before, discount):
    return Decimal(Decimal(100) * discount / before) if discount is not None and before is not None else None


@journal_save_delete
class ReservationPackage(models.Model):
    Meta = named_meta('Бронирование пакета', 'ReservationPackage')

    date_from = models.DateTimeField(help_text='Дата начала', null=True)
    date_to = models.DateTimeField(help_text='Дата окончания', null=True)

    branding = models.BooleanField(default=False, help_text='брендинг', null=False)
    # design = models.BooleanField(default=False, null=False)  # TODO: help_text

    reservation_type = models.ForeignKey(
        ReservationType,
        on_delete=models.DO_NOTHING,
        related_name='reservation_package',
        null=True
    )

    project = models.ForeignKey(
        Project,
        on_delete=models.SET_NULL,
        related_name='reservation_package',
        null=True
    )

    appendix = models.ForeignKey(
        'Appendix',
        help_text='Бронирования пакетов -> Приложение к договору',
        related_name='packages_reservations',
        blank=True,
        null=True,
        on_delete=models.SET_NULL
    )

    # # TODO: Проверить: в смете может быть несколько бронирований, но одно бронирование участвует только в одной смете
    # estimate = models.ForeignKey('Estimate', related_name='reservation_package', null=True,
    #                              on_delete=models.SET_NULL)  # SET_NULL т.к. для бронирования может и не быть сметы
    # TODO: У бронирования должна быть связь с адрессной книгой

    package = models.ForeignKey(
        'Package',
        help_text='Бронирования Пакета -> Пакет',
        related_name='reservation_packages',
        null=True,
        on_delete=models.SET_NULL
    )

    def send_mail_about_canceled(self):
        subject = f'Ваша бронь отменена'
        message = f'Бронь пакета {self.package.title} ({self.package.city}) с {self.date_from.date()} по {self.date_to.date()} была отменена'
        from_email = settings.EMAIL_HOST_USER
        if self.project:
            to_emails = [self.project.client.email]
            send_mail(subject, message, from_email, to_emails)

    def save(self, *args, **kwargs):
        related_reservations = Reservation.objects.filter(
            reservation_package=self
        )
        if self.reservation_type.ikey == ReservationType.ReservationTypeInteger.CANCELLED:
            self.send_mail_about_canceled()
            for res in related_reservations:
                res.reservation_type = ReservationType.objects.get(
                    ikey=ReservationType.ReservationTypeInteger.CANCELLED
                )
                res.save()
        if self.reservation_type.ikey == ReservationType.ReservationTypeInteger.SALED:
            for res in related_reservations:
                res.reservation_type=ReservationType.objects.get(
                    ikey=ReservationType.ReservationTypeInteger.SALED
                )
                res.save()
            conflict_package_reservations = ReservationPackage.objects.filter(
                (Q(date_from__lte=self.date_from) & Q(date_to__gte=self.date_from)) |
                (Q(date_from__lte=self.date_to) & Q(date_to__gte=self.date_to)) |
                (Q(date_from__gte=self.date_from) & Q(date_to__lte=self.date_to)),
                package=self.package,
                reservation_type__ikey=ReservationType.ReservationTypeInteger.RESERVED)
            if self.pk is not None:
                conflict_package_reservations = conflict_package_reservations.exclude(pk=self.pk)
            for res in conflict_package_reservations:
                res.reservation_type=ReservationType.objects.get(
                    ikey=ReservationType.ReservationTypeInteger.CANCELLED
                )
                res.save()

        super().save(*args, **kwargs)


@journal_save_delete
class BrandImage(models.Model):
    Meta = named_meta('Изображение бренда', 'BrandImage')

    img = models.ImageField(upload_to='brand', help_text='Изображение', null=True, max_length=IMAGE_URL_MAX_LENGTH)
    active = models.BooleanField(default=False, null=False)  # TODO: help_text

    brand = models.ForeignKey(Brand, help_text='Бренд', related_name='brand_images', null=True,
                              on_delete=models.CASCADE)


@journal_save_delete
class LastNumYearProject(PostgresModel):
    __metaclass__ = named_meta('Последний порядковый номер проекта', 'Последний порядковый номер проекта',
                               unique_together=['year'])

    year = models.SmallIntegerField(help_text='Год начала проекта', null=False, unique=True)
    number = models.BigIntegerField(null=True, help_text='Последний порядковый номер проекта в году')
