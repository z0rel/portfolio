from collections import OrderedDict

from graphene import ObjectType, ID, List, String, DateTime, Field, Int, Boolean

from ..m_mobile.graphql_answer_main import FIELDS_CONSTRUCTION_SIDE, MOUNTING_FIELDS


MountingProjectCardMountingTaskItem = type(
    'MountingProjectCardMountingTaskItem',
    (ObjectType,),
    OrderedDict(
        [
            ('Meta', type('MountingProjectCardMountingTaskItem.Meta', (), {'description': 'Монтажная задача'})),
            *MOUNTING_FIELDS.items(),
            ('mounting_task_title', String(description='Название типа задачи'))
        ]
    ),
)


class MountingDesignProjectCard(ObjectType):
    class Meta:
        description = 'Дизайн, заданный для монтажной задачи (экран проекта монтажей)'

    id = ID(description='Идентификатор дизайна')
    img = String(description='URL-адрес изображения дизайна монтажа')
    startedAt = DateTime(description='Дата начала действия дизайна')
    title = String(description='Название дизайна')


MOUNTING_PROJECT_CARD_FIELD = OrderedDict([
    ('id', ID(description='Идентификатор бронирования')),
    *FIELDS_CONSTRUCTION_SIDE.items(),  # информация о конструкции

    ('comments', String(description='Комментарии')),

    ('reservation__date_from', DateTime(description='Начало бронирования')),
    ('reservation__date_to', DateTime(description='Окончание бронирования')),
    ('reservation__branding', Boolean(description='Наличие брендинга в бронировании')),

    ('min_date_mounting', DateTime(description='Наиболее ранняя дата монтажа')),
    ('max_date_mounting', DateTime(description='Наиболее поздняя дата монтажа')),
    ('min_date_unmounting', DateTime(description='Наиболее ранняя дата демонтажа')),
    ('max_date_unmounting', DateTime(description='Наиболее поздняя дата демонтажа')),
    ('min_photo_date', DateTime(description='Наиболее ранняя дата фотоотчета')),
    ('min_photo_additional_day_date', DateTime(description='Наиболее ранняя дата дневного фотоотчета')),
    ('min_photo_additional_night_date', DateTime(description='Наиболее ранняя дата ночного фотоотчета')),
    ('crews', String(description='Экипажи')),

    ('unmounting_design', Field(MountingDesignProjectCard, description='Демонтируемый дизайн')),
    ('previous_design', Field(MountingDesignProjectCard, description='Предыдущий дизайн')),
    ('current_design', Field(MountingDesignProjectCard, description='Текущий дизайн')),

    # Структурные поля
    ('mounting_tasks_total', Int(description='Задачи по монтажу/демонтажу - длина списка')),
    ('mounting_tasks',
     List(MountingProjectCardMountingTaskItem, description='Задачи по монтажу/демонтажу')),
    ('additional_photo_day_total', Int(description='Задачи по дополнительному дневному фотоотчету - длина списка')),
    ('additional_photo_day',
     List(MountingProjectCardMountingTaskItem, description='Задачи по дополнительному дневному фотоотчету')),
    ('additional_photo_night_total', Int(description='Задачи по дополнительному ночному фотоотчету - длина списка')),
    ('additional_photo_night',
     List(MountingProjectCardMountingTaskItem, description='Задачи по дополнительному ночному фотоотчету')),
])


AnswerMountingProjectCardField = type(
    'AnswerMountingProjectCardField',
    (ObjectType,),
    OrderedDict([
        ('Meta', type('AnswerMountingProjectCardField.Meta', (), {'description': 'Бронирование с данными монтажа'})),
        *MOUNTING_PROJECT_CARD_FIELD.items(),
    ]),
)
