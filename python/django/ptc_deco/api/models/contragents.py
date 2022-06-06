from django.db import models
from django.utils import timezone

from .utils import named_meta, journal_save_delete
from .geolocations import District, Postcode, Addresses
from .projects import Brand
from .agency_comission import AgencyCommission, DiscountMixinPartnerAndProject


@journal_save_delete
class WorkingSector(models.Model):
    Meta = named_meta('Сектор деятельности', 'WorkingSector')

    title = models.CharField(max_length=256, help_text='Наименование', unique=True)
    description = models.TextField(help_text='Описание')

# Тип контрагента (выбирается рекламодатель, рекламное агентство, поставщик.
# При выборе рекламного агентства к контрагенту можно привязывать прямого рекламодателя)
@journal_save_delete
class PartnerType(models.Model):
    Meta = named_meta('Тип контрагента', 'PartnerType')

    title = models.CharField(max_length=256, help_text='Тип контрагента', unique=True)


@journal_save_delete
class ClientType(models.Model):
    Meta = named_meta('Тип клиента', 'ClientType', unique_together=[('title', 'description')])

    title = models.CharField(max_length=256, help_text='Название')
    description = models.CharField(max_length=256, help_text='Описание')


# Добавляются поля "Процент скидки на налог" и "Процент скидки по прайсу"
@journal_save_delete
class Partner(DiscountMixinPartnerAndProject):
    Meta = named_meta('Контрагент', 'Partner',
                      unique_together=[
                          'title', 'bin_number', 'bank_recipient', 'legal_address', 'actual_address', 'partner_type',
                          'checking_account'
                      ])

    title = models.CharField(max_length=256, help_text='Наименование', null=True)
    comment = models.TextField(help_text='Комментарий', null=True)
    bin_number = models.CharField(max_length=32, help_text='БИН', null=True)  # Что это?
    foreign_partner = models.BooleanField(default=False, help_text='Иностранный контрагент', null=False)

    bank_recipient = models.CharField(max_length=256, help_text='Банк получателя', null=True)
    iik = models.CharField(max_length=32, help_text='ИИК', null=True)
    bik = models.CharField(max_length=32, help_text='БИК', null=True)
    kbe = models.CharField(max_length=4, help_text='КБЕ', null=True)
    checking_account = models.CharField(max_length=128, help_text='Расчетный счет', null=True)

    nds_certificate_number = models.CharField(max_length=64, help_text='Номер свидетельства о постановке на НДС', null=True)

    email = models.EmailField(max_length=254, help_text='Почтовый адрес', null=True)

    created_at = models.DateTimeField(auto_now_add=True, help_text='Дата создания', null=True)
    updated_at = models.DateTimeField(auto_now=True, help_text='Дата редактирования', null=True)

    is_nonrts_owner = models.BooleanField(null=True, help_text='Является владельцем конструкций НОН РТС')

    legal_address = models.ForeignKey(
        Addresses,
        help_text='Контрагенты -> Юридический адрес',
        related_name='legal_address_partners',
        null=True,
        on_delete=models.SET_NULL
    )

    legal_address_postcode = models.ForeignKey(
        Postcode,
        help_text='Контрагенты -> Юридический адрес - Почтовый индекс',
        related_name='legal_postcode_partners',
        null=True,
        on_delete=models.SET_NULL
    )

    actual_address = models.ForeignKey(
        Addresses,
        help_text='Контрагенты -> Фактический адрес',
        related_name='actual_address_partners',
        null=True,
        on_delete=models.SET_NULL
    )

    actual_address_postcode = models.ForeignKey(
        Postcode,
        help_text='Контрагенты -> Фактический адрес - Почтовый индекс',
        related_name='actual_postcode_partners',
        null=True,
        on_delete=models.SET_NULL
    )

    # К одному рекламодателю может быть присвоено несколько секторов.
    # Сектор можно удалить из рекламодателя, выделив его и нажав на кнопку «Удалить»
    working_sectors = models.ManyToManyField(
        WorkingSector,
        help_text='Контрагенты -> Секторы деятельности',
        related_name='partners',
        blank=True
    )
    partner_type = models.ForeignKey(
        PartnerType,
        help_text='Контрагенты -> Тип контрагента',
        related_name='partners',
        null=True,
        on_delete=models.SET_NULL
    )
    client_type = models.ForeignKey(
        ClientType,
        help_text='Контрагенты -> Тип клиента',
        related_name='partners',
        null=True,
        on_delete=models.SET_NULL
    )
    district = models.ForeignKey(
        District,
        help_text='Контрагенты -> Район',
        related_name='partners',
        null=True,
        on_delete=models.SET_NULL
    )
    brands = models.ManyToManyField(
        Brand,
        help_text='Контрагенты <-> Связанные бренды',
        related_name='partners',
        blank=True  # !! Не удалять, иначе поле станет обязательным в API, warning - игнорировать
    )
    advertisers = models.ManyToManyField(
        'Partner', 
        help_text='Контрагенты <-> Связанные рекламодатели', 
        related_name='advertiser_clients',  # клиенты рекламодателя
        blank=True  # !! Не удалять, иначе поле станет обязательным в API, warning - игнорировать
    )

    agency_commission = models.ForeignKey(
        AgencyCommission,
        help_text='Контрагенты -> Агентская комиссия',
        null=True,
        on_delete=models.SET_NULL,
        related_name='partners',
    )

    legal_entity_payee_agency_comission = models.ForeignKey(
        'Partner',
        help_text='Контрагенты -> Юридическое лицо, получатель агентской комиссси',
        null=True,
        on_delete = models.SET_NULL,
        related_name='senders_of_agency_comission',
    )

    def save(self, *args, **kwargs):
        if not self.id:
            self.created_at = timezone.now()
        self.updated_at = timezone.now()

        return super(Partner, self).save(*args, **kwargs)

    def __str__(self):
        return f'Контрагент №0000{ self.id } - { self.title }'


# Структура страницы с контрагентом
@journal_save_delete
class ContactPerson(models.Model):
    Meta = named_meta('Контактное лицо', 'ContactPerson')
    # Контактная информация: ФИО, телефон, e-mail.

    name = models.CharField(max_length=256, help_text='ФИО')
    phone = models.CharField(max_length=64, help_text='Телефон')
    email = models.CharField(max_length=128, help_text='E-mail')

    partner = models.ForeignKey(
        Partner,
        help_text='Контактные лица -> Контрагент',
        related_name='contact_persons',
        null=True,
        on_delete=models.SET_NULL
    )
