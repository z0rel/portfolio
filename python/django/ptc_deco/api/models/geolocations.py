import re
from django.db import models

from .utils import IMAGE_URL_MAX_LENGTH
from .utils import named_meta, journal_save_delete


@journal_save_delete
class Country(models.Model):
    Meta = named_meta('Страна', 'Country')

    title = models.CharField(max_length=128, unique=True, null=True, help_text='Страна')


@journal_save_delete
class City(models.Model):
    Meta = named_meta('Город', 'City', unique_together=['title', 'country'])

    title = models.CharField(max_length=64, help_text='Название', null=True)
    country = models.ForeignKey(
        Country, help_text='Города -> Страна', related_name='cities', null=False, on_delete=models.CASCADE
    )


@journal_save_delete
class District(models.Model):
    Meta = named_meta('Район', 'District', unique_together=['title', 'city'])

    title = models.CharField(max_length=128, help_text='Название', null=True)
    city = models.ForeignKey(
        City, help_text='Районы -> Город', related_name='districts', null=False, on_delete=models.CASCADE
    )
    RXP = re.compile('^[0-9\\s]+$')

    def __str__(self):
        return str((self.id, self.title, 'city_id', self.city_id))

    def save(self, *args, **kwargs):
        if self.title:
            if self.RXP.match(self.title):
                raise Exception(f'Название района {self.title} не может быть почтовым кодом')
        super().save(*args, **kwargs)

    # related
    # Postcode postcodes
    # Partner partners


@journal_save_delete
class Postcode(models.Model):
    Meta = named_meta('Почтовый индекс', 'Postcode', unique_together=['title', 'district'])

    title = models.CharField(max_length=32, help_text='Название', null=True)
    district = models.ForeignKey(
        District, help_text='Почтовые индексы -> Район', related_name='postcodes', null=False, on_delete=models.CASCADE
    )

    def __str__(self):
        return str((self.id, self.title, 'district_id', self.district_id))

    # related:
    # Partner (legal_address_postcode, actual_postcode)
    # Location (postcode)
    # Addresses (postcode)
    #


@journal_save_delete
class PurposeLocation(models.Model):
    Meta = named_meta('Целевое назначение местоположения', 'PurposeLocation')

    title = models.CharField(max_length=2048, null=False, unique=True)


# Наличие документов на землю. Статус автоматически прописывается программой, либо присваивается пользователем
#  вручную. Возможные статусы, в зависимости от стадии оформления аренды земельного участка:
#  1. Оформлен – земельный участок оформлен, когда срой действия аренды земельного участка не просрочен и
#     в программу подгружены все правоустанавливающие документы (выписка из постановления акимата, гос акт,
#     договор аренды, земельный проект).
#  2. На оформлении – земельный участок ранее был оформлен, но срок действия аренды участка истек и на данный
#     момент участок находится на пролонгации документов. Также у нас бывают разные потоки оформления документов,
#     хотелось бы сделать возможность подкатегории для этого статуса.
#  3. Нет документов – данный статус присваивается, в случае если конструкция смонтирована впервые и ранее
#     аренда данного земельного участка не оформлялась.
#  4. Невозможно оформить – данный статус присваивается, в случае если земельный участок оформлен на третьих
#     лиц (например, Сениор на Магнуме, Остановка в Аэропорту). Статус может быть отмечен галочкой юристами вручную,
#     но программа будет показывать уведомление о том, что нужно подгрузить файл.


@journal_save_delete
class RegistrationStatusLocation(models.Model):
    Meta = named_meta(
        'Статус оформления местоположения', 'RegistrationStatusLocation', unique_together=['title', 'subcategory']
    )

    title = models.CharField(max_length=64, help_text='Статус оформления', null=False)
    subcategory = models.CharField(max_length=512, help_text='Подкатегория оформления', null=True)


class Addresses(models.Model):
    Meta = named_meta('Адресный справочник', 'Addresses', unique_together=['address', 'postcode'])

    address = models.CharField(max_length=512, null=False, help_text='Адрес')

    postcode = models.ForeignKey(
        Postcode, help_text='Адреса -> Почтовый индекс', related_name='addresses', null=True, on_delete=models.SET_NULL
    )

    def __str__(self):
        return str((self.address, 'postcode_id', self.postcode))

    # related:
    # Location (locations by marketing address, locations by actual address)\
    # Partners (legal address)
    # Partners (actual address)


@journal_save_delete
class Location(models.Model):
    Meta = named_meta(
        'Местоположение',
        'Location',
        unique_together=[[
            'marketing_address',
            'legal_address',
            'postcode',
            'purpose_location',
            'cadastral_number',
            'resolution_number',
            'area_act',
            'family_construction',
        ]],
        index_together=[['cadastral_number']]
    )

    has_area = models.BooleanField(help_text='Наличие земли', default=True, null=False)

    area = models.DecimalField(max_digits=20, help_text='Площадь (га)', null=True, decimal_places=10)

    cadastral_number = models.CharField(max_length=128, help_text='Кадастровый номер', null=True)
    comment = models.TextField(max_length=512, help_text='Комментарий', null=True)

    resolution_number = models.CharField(max_length=128, help_text='Номер постановления от Акимата', null=True)
    resolution_number_date = models.DateTimeField(help_text='Дата постановления от Акимата', null=True)
    area_act = models.CharField(max_length=128, help_text='Номер гос акта на землю', null=True)
    area_act_date = models.DateTimeField(help_text='Дата гос акта на землю', null=True)

    rent_contract_number = models.CharField(max_length=128, help_text='Номер договора', null=True)
    rent_contract_start = models.DateTimeField(help_text='Дата начала договора', null=True)
    rent_contract_end = models.DateTimeField(help_text='Дата окончания договора', null=True)

    rent_contract_created_at = models.DateTimeField(help_text='Регистрация договора', null=True)
    created_at = models.DateTimeField(auto_now_add=True, help_text='Дата создания', null=True)
    updated_at = models.DateTimeField(auto_now=True, help_text='Дата обновления', null=True)

    is_nonrts_location = models.BooleanField(
        default=False, help_text='Местоположение конструкций, не принадлежащих РТС'
    )

    document = models.ImageField(
        upload_to='location',
        null=True,
        help_text='Правоустанавливающие документы на земельный участок',
        max_length=IMAGE_URL_MAX_LENGTH,
    )

    is_archive = models.BooleanField(default=False, help_text='В архиве')

    marketing_address = models.ForeignKey(
        Addresses,
        help_text='Местоположения -> Маркетинговый адрес',
        related_name='locations_by_maketing_adress',
        null=True,
        on_delete=models.SET_NULL,
    )
    legal_address = models.ForeignKey(
        Addresses,
        help_text='Местоположения -> Юридический адрес',
        related_name='locations_by_actual_adress',
        null=True,
        on_delete=models.SET_NULL,
    )
    # По почтовому индексу выводится город и район
    postcode = models.ForeignKey(
        Postcode,
        help_text='Местоположения -> Почтовый индекс',
        related_name='locations',
        null=True,
        on_delete=models.SET_NULL,
    )
    purpose_location = models.ForeignKey(
        PurposeLocation,
        related_name='locations',
        help_text='Местоположения -> Целевое назначение местоположения',
        null=True,
        on_delete=models.SET_NULL,
    )
    # Не раскрывать в API, мутации через ID, т.к. статусы должны быть уникальны
    registration_status_location = models.ForeignKey(
        RegistrationStatusLocation,
        help_text='Местоположения -> Статус оформления земельного участка',
        related_name='locations',
        null=True,
        on_delete=models.SET_NULL,
    )

    family_construction = models.ForeignKey(
        'FamilyConstruction',
        help_text='Местоположения -> Семейство устанавливаемых конструкций в данном месте',
        related_name='locations',
        null=True,
        on_delete=models.SET_NULL,
    )

    row_idx = models.IntegerField(null=True)
    construction_row_idx = models.IntegerField(null=True)

    def __str__(self):
        return str(
            (
                self.id,
                self.row_idx,
                self.construction_row_idx,
                self.cadastral_number,
                self.marketing_address,
                self.legal_address,
                self.has_area,
                self.area,
                self.cadastral_number,
                self.comment,
                self.resolution_number,
                self.resolution_number_date,
                self.area_act,
                self.area_act_date,
                self.rent_contract_number,
                self.rent_contract_start,
                self.rent_contract_end,
                self.rent_contract_created_at,
                self.created_at,
                self.updated_at,
                self.is_nonrts_location,
                self.document,
                self.is_archive,
                self.postcode,
                self.purpose_location,
                self.registration_status_location,
                self.family_construction,
            )
        )
