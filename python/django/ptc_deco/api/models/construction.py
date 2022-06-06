import re

from django.contrib.postgres.fields import ArrayField
from django.db import models
from django.utils import timezone
from .utils.history import HistoryModel

from .utils import named_meta, journal_save_delete, IMAGE_URL_MAX_LENGTH
from .geolocations import Location, District, Addresses, Postcode


@journal_save_delete
class FamilyConstruction(models.Model):
    Meta = named_meta('Семейство конструкции', 'FamilyConstruction')

    title = models.CharField(max_length=128, help_text='Наименование', null=True, unique=True)


@journal_save_delete
class UnderFamilyConstruction(models.Model):
    Meta = named_meta('Подсемейство конструкции', 'UnderFamilyConstruction', unique_together=['title', 'family'])

    title = models.CharField(max_length=128, help_text='Наименование', null=True)
    family = models.ForeignKey(
        FamilyConstruction,
        help_text='Подсемейства -> Семейство конструкции',
        related_name='underfamilies',
        null=False,
        on_delete=models.CASCADE,
    )


@journal_save_delete
class ModelConstruction(models.Model):
    Meta = named_meta('Модель конструкции', 'ModelConstruction', unique_together=['title', 'underfamily'])

    title = models.CharField(max_length=128, null=True)
    underfamily = models.ForeignKey(
        UnderFamilyConstruction,
        help_text='Модели -> Подсемейство конструкции',
        related_name='models',
        null=False,
        on_delete=models.CASCADE,
    )


@journal_save_delete
class Format(models.Model):
    Meta = named_meta('Формат', 'Format', unique_together=['title', 'code', 'model'])

    title = models.CharField(max_length=128, help_text='Наименование формата', null=True)
    code = models.CharField(max_length=16, help_text='Код формата', null=True)

    model = models.ForeignKey(
        ModelConstruction,
        help_text='Форматы -> Модель конструкции',
        related_name='formats',
        null=False,
        on_delete=models.CASCADE,
    )


MAP_RUSSIAN_CODES = {
    'А': 'A',
    'А1': 'A1',
    'А2': 'A2',
    'А3': 'A3',
    'А4': 'A4',
    'В': 'B',
    'В1': 'B1',
    'В2': 'B2',
    'В3': 'B3',
    'В4': 'B4',
}


def tr_coded_title(s):
    if not s:
        return s, None
    arr = s.split(' ')
    if len(arr) > 1:
        c = arr[-1]
        c = MAP_RUSSIAN_CODES.get(c, c)
        arr[-1] = c
        s = ' '.join(arr)
        return s, c
    return s, None


def tr_code_on_model(model_instance):
    if model_instance.title:
        model_instance.title, code = tr_coded_title(model_instance.title)
        if model_instance.code:
            model_instance.code = MAP_RUSSIAN_CODES.get(model_instance.code, model_instance.code)
        elif code:
            model_instance.code = code
    elif model_instance.code:
        model_instance.code = MAP_RUSSIAN_CODES.get(model_instance.code, model_instance.code)


@journal_save_delete
class Side(models.Model):
    Meta = named_meta('Сторона', 'Side', unique_together=['title', 'size', 'format'])

    title = models.CharField(max_length=128, null=True, help_text='Наименование стороны')
    size = models.CharField(max_length=32, null=True, help_text='Размер стороны')
    code = models.CharField(max_length=8, null=True, help_text='Буквенный код стороны')

    format = models.ForeignKey(
        Format, help_text='Стророны -> Формат', related_name='sides', null=False, on_delete=models.CASCADE
    )

    def save(self, *args, **kwargs):
        tr_code_on_model(self)

        if self.code is None:
            raise Exception(f'Код для стороны {self.title} не может быть Null')
        super().save(*args, **kwargs)


@journal_save_delete
class AdvertisingSide(models.Model):
    Meta = named_meta('Рекламная сторона', 'AdvertisingSide', unique_together=['title', 'side'])

    title = models.CharField(max_length=128, null=True, help_text='Наименование рекламной стороны')
    code = models.CharField(max_length=8, null=True, help_text='Буквенный код рекламной стороны')

    side = models.ForeignKey(
        Side,
        help_text='Рекламные стороны -> Сторона',
        related_name='advertising_sides',
        null=False,
        on_delete=models.CASCADE,
    )
    RXP = re.compile('.* ([^ \t\v]+)')

    def save(self, *args, **kwargs):
        tr_code_on_model(self)
        if self.code is None:
            raise Exception(f'Код для рекламной стороны {self.title} не может быть Null')
        super().save(*args, **kwargs)


class Obstruction(models.Model):
    Meta = named_meta('Помеха', 'Obstruction')

    title = models.CharField(max_length=256, null=False, help_text='Помеха', unique=True)


class TechProblems(models.Model):
    Meta = named_meta('Техническая проблема', 'TechProblems')
    # Разбита створка, Разбито стекло, Нет напряжения, Не работает часть диодов, Не работают все диоды
    # Заменить амортизатор, Не работает двигатель, Панель (вариаторы), Ремонт постеродержателя, Замена лавочки, ДТП
    # Если в технической проблеме указан критерий «Нет напряжения» или «Не работают все диоды», статус «Не горит» в
    # разделе статуса по подключению проставляется автоматически. Так же, при выборе в статусе подключения «Не горит»,
    # пользователю сразу предлагается выбрать одну или обе из указанных выше критериев в разделе технических проблемах.
    # Рядом с технической проблемой должно быть поле комментария (до 20 символов), относящееся к этому параметру.
    title = models.CharField(max_length=256, null=True, help_text='Наименование технической проблемы', unique=True)
    comment = models.TextField(null=True, help_text='Комментарий к технической проблеме')


# Сначала создаётся местоположение – оно же земля,
# к местоположению присваивается конструкция,
# к конструкции присваиваются особенности (параметры и стороны).

# @journal_save_delete


class Construction(models.Model):
    Meta = named_meta(
        'Конструкция',
        'Construction',
        unique_together=[
            ['tech_invent_number', 'buh_invent_number', 'location', 'model'],
        ],
        index_together=[
            ['tech_invent_number', 'buh_invent_number', 'location', 'model'],
            ['tech_invent_number'],
            ['buh_invent_number'],
            ['location'],
            ['model'],
        ],
    )
    row_idx = models.IntegerField(null=True)

    # Код состоит из 050001.00361, где 050001 – почтовый индекс, он же код района,
    # 00361 – порядковый номер конструкции в этом районе.
    # Инициализация сделана триггером
    num_in_district = models.BigIntegerField(null=True, help_text='Порядковый номер конструкции в районе')

    creation_date = models.DateTimeField(null=True, help_text='Дата создания', blank=True)

    coordinates = models.CharField(max_length=256, help_text='Координаты', null=True)

    back_comment = models.TextField(null=True, help_text='Комментарий')
    tech_comment = models.TextField(null=True, help_text='Комментарий тех. отдела')

    tech_invent_number = models.CharField(max_length=128, null=True, help_text='Инвентарный номер - Техотдел')
    # Инвентарный номер в соответствии с 1С
    buh_invent_number = models.CharField(max_length=128, null=True, help_text='Инвентарный номер - 1C')

    # На каждой конструкции стоит сим карта с номером телефона, которая отправляет смс при неисправности
    # 64 т.к. телефонов может быть несколько
    tech_phone_construction = models.CharField(max_length=64, null=True, help_text='Номер телефона конструкции')

    # Статус по подключению (горит/не горит).
    # Статус «не горит» отображается в справочнике рекламных сторон, однако бронь на этом адресе можно выставлять.
    # Также, статус НГ (не горит) может быть присвоен (или убран) и в общем списке всех конструкций сразу к нескольким
    # из них; пользователь находит конструкции по параметрам и, выбрав несколько или все из них, присваивает новый
    # статус. Изменения по НГ отображаются сразу.
    # TODO: повесить триггер на create и update
    status_connection = models.BooleanField(default=False, null=False, help_text='Статус по подключению')

    status_availability = models.BooleanField(default=False, null=False, help_text='Статус доступности конструкции')

    # TODO: вынести в таблицу или сделать подсказки стандартных вариантов на фронтэнде
    # Дерево, Столб, Сан.обрезка, Знак, Светофор, Урна, Конкурент, Другая конструкция

    obstruction = models.ForeignKey(
        Obstruction,
        help_text='Конструкции -> Помеха',
        related_name='constructions',
        null=True,
        on_delete=models.SET_NULL,
    )

    tech_problem = models.ManyToManyField(
        TechProblems,
        help_text='Конструкция -> Технические проблемы',
        related_name='constructions',
        blank=True,
    )
    tech_problem_all_comment = models.CharField(max_length=(1024*2), null=True, help_text='Список комментариев к тех. проблемам')

    photo = models.ImageField(
        upload_to='construction', null=True, help_text='Изображение', max_length=IMAGE_URL_MAX_LENGTH
    )

    active = models.BooleanField(default=False, help_text='Активная или демонтировананя', null=False)
    # Дата создания (по умолчанию сразу отображается текущий день, дату можно поменять)
    created_at = models.DateTimeField(auto_now_add=True, help_text='Дата создания', null=True)
    updated_at = models.DateTimeField(auto_now=True, help_text='Дата обновления', null=True)

    presentation_url = models.CharField(
        max_length=256, null=True, help_text='Ссылка на сайт с презентацией конструкции'
    )
    is_archive = models.BooleanField(default=False, help_text='В архиве')

    # TODO: Добавить триггер контроля целостности - модель стороны конструкции должна совпадать с моделью конструкции
    model = models.ForeignKey(
        ModelConstruction,
        help_text='Конструкции -> Модель конструкции',
        related_name='constructions',
        null=True,
        on_delete=models.SET_NULL,
    )

    # Параметры заполнения информации об экипаже в базе – номер экипажа, имя человека и город
    crew = models.ForeignKey(
        'Crew', help_text='Конструкции -> Экипаж', related_name='constructions', null=True, on_delete=models.SET_NULL
    )
    crews_has_special_for_sides = models.BooleanField(
        null=True, help_text='Сторонам конструкции присвоены ' 'специализированные экипажи'
    )

    # На одном местоположении может быть несколько действующих и демонтированных конструкций.
    location = models.ForeignKey(
        Location,
        help_text='Конструкции -> Местоположение',
        related_name='constructions',
        null=True,
        on_delete=models.SET_NULL,
    )

    # Если не установлен, то владелец - РТС
    is_nonrts = models.BooleanField(default=False, help_text='Конструкция НОН РТС', null=False)

    nonrts_owner = models.ForeignKey(
        'Partner',
        help_text='Конструкции -> Владелец НОН РТС',
        related_name='owned_nonrts_constructions',
        null=True,
        on_delete=models.SET_NULL,
    )
    nonrts_owner_comment = models.CharField(max_length=256, null=True, help_text='Коментарий о владельце НОН РТС')

    marketing_address = models.ForeignKey(
        Addresses,
        help_text='Местоположения -> Маркетинговый адрес',
        related_name='constructions_by_maketing_adress',
        null=True,
        on_delete=models.SET_NULL,
    )

    legal_address = models.ForeignKey(
        Addresses,
        help_text='Местоположения -> Юридический адрес',
        related_name='constructions_by_actual_adress',
        null=True,
        on_delete=models.SET_NULL,
    )

    postcode = models.ForeignKey(
        Postcode,
        help_text='Местоположения -> Почтовый индекс',
        related_name='constructions',
        null=True,
        on_delete=models.SET_NULL,
    )

    def save(self, *args, **kwargs):
        previous = Construction.objects.filter(id=self.id).first()
        super(Construction, self).save(*args, **kwargs)
        ConstructionHistory.save_diff(current=self, previous=previous)

    def delete(self):
        ConstructionHistory.close(self)
        super(Construction, self).delete()

    def __str__(self):
        return f"""Construction object {self.id}, row_idx: {self.row_idx} model {self.model_id} {self.model.title} address legal: {
        self.location.legal_address.address if self.location and self.location.legal_address else None
        }, address market: {self.location.marketing_address.address if self.location and self.location.marketing_address else None}"""


ConstructionHistory = HistoryModel(name='construction_history',
                                   app_label='api',
                                   module='ptcbackend',
                                   target_model=Construction)


class ConstructionFormats(models.Model):
    Meta = named_meta(
        'Множественные форматы конструкций',
        'ConstructionFormats',
        unique_together=[
            ['construction', 'format'],
        ],
    )

    construction = models.ForeignKey(
        Construction,
        help_text='Коды форматов конструкций -> Конструкция',
        related_name='formats',
        null=True,
        on_delete=models.CASCADE,
    )

    format = models.ForeignKey(
        Format,
        help_text='Коды форматов конструкций -> Форматы',
        related_name='constructions_for_format',
        null=True,
        on_delete=models.CASCADE,
    )

    count = models.IntegerField(
        default=0, help_text='Количество сторон с заданным форматом в конструкции', null=False, blank=True
    )


@journal_save_delete
class PurposeSide(models.Model):
    Meta = named_meta('Назначение стороны', 'PurposeSide')

    title = models.CharField(max_length=512, null=False, unique=True)


# Информация о помехах доступна при просмотре справочника рекламных сторон.

# class FieldChangesDetector:
#     def __init__(self, model: models.Model, instance):
#         self.model = model
#         self.fields = self.model._meta.fields
#         self.attr = {field.name: getattr(instance, field.name) for field in self.fields}
#
#     def


@journal_save_delete
class ConstructionSide(models.Model):
    Meta = named_meta(
        'Сторона конструкции', 'ConstructionSide', unique_together=[['construction_id', 'advertising_side_id']]
    )

    created_at = models.DateTimeField(auto_now_add=True, help_text='Дата создания', null=True)
    updated_at = models.DateTimeField(auto_now=True, help_text='Дата обновления', null=True)

    availability_side = models.BooleanField(default=True, help_text='Статус доступности стороны', null=False)
    is_archive = models.BooleanField(default=False, help_text='В архиве')

    # На конструкцию можно поставить ограничение по продажам. Конструкция при этом останется в наличии, но на нее
    # невозможно будет поставить бронь. Ограничение на бронирование особым образом отображается в справочнике рекламных
    # сторон. Ограничение может быть снято или поставлено в любой момент. Если ограничение ставится на уже
    # забронированную сторону, пользователю, высылается уведомление о том, что забронированная конструкция находится
    # в недоступности с текущего времени.
    sale_constraint = models.BooleanField(
        default=False, help_text='Ограничение стороны конструкции по продажам', null=False
    )

    # Сторона конструкции принадлежит только одной конструкции, но у конструкции может быть несколько сторон
    construction = models.ForeignKey(
        Construction,
        help_text='Стророны конструкции -> Конструкция',
        related_name='owned_sides',
        null=True,
        on_delete=models.SET_NULL,
    )
    # По рекламной стороне можно вывести сторону, формат, модель, семейство и подсемейство
    advertising_side = models.ForeignKey(
        AdvertisingSide,
        help_text='Стророны конструкции -> Рекламная сторона',
        related_name='construction_sides',
        null=True,
        on_delete=models.SET_NULL,
    )
    purpose_side = models.ForeignKey(
        PurposeSide,
        help_text='Стророны конструкции -> Назначение стороны',
        related_name='construction_sides',
        null=True,
        on_delete=models.SET_NULL,
    )

    package = models.ForeignKey(
        'Package',
        help_text='Стророны конструкции -> Пакет',
        related_name='construction_sides',
        null=True,
        on_delete=models.SET_NULL,
    )

    crew = models.ForeignKey(
        'Crew',
        help_text='Конструкции -> Экипаж',
        related_name='construction_sides',
        null=True,
        on_delete=models.SET_NULL,
    )

    def save(self, *args, **kwargs):
        if not self.id:
            self.created_at = timezone.now()
        self.updated_at = timezone.now()
        previous = ConstructionSide.objects.filter(id=self.id).first()
        super(ConstructionSide, self).save(*args, **kwargs)
        ConstructionSideHistory.save_diff(current=self, previous=previous)

    def delete(self):
        ConstructionSideHistory.close()
        super(ConstructionSide, self).delete()

    def __str__(self):
        return f"""Сторона конструкции - {self.id} construction: {self.construction_id} adv: {self.advertising_side_id
        }, format: {self.advertising_side.side.format.id if self.advertising_side is not None else ""
        } {self.advertising_side.side.format.title if self.advertising_side is not None else ""
        }"""


ConstructionSideHistory = HistoryModel(name='construction_side_history',
                                       app_label='api',
                                       module='ptcbackend',
                                       target_model=ConstructionSide)


@journal_save_delete
class LastSerialNumber(models.Model):
    Meta = named_meta('Последний порядковый номер конструкции в районе', 'LastSerialNumber')

    number = models.BigIntegerField(null=True, help_text='Последний порядковый номер конструкции')

    district = models.OneToOneField(
        District,
        help_text='Последний порядковый номер конструкции -> Район',
        related_name='last_serial_numbers',
        on_delete=models.CASCADE,
    )
