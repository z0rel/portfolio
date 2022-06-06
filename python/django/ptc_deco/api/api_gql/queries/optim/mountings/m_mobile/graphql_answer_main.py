from collections import OrderedDict

from graphene import ObjectType, Boolean, Field, String, DateTime, ID, Int, List, Date


from .graphql_answer_entities import (
    MountingDesignField, MountingCrewField, MountingProjectField, ConstructionSideInfo, MOUNTING_RANGE_ENUM
)

class MountingPhotos(ObjectType):
    class Meta:
        description = 'Фотоотчеты'

    photo = String(description='URL фотографии фотоотчета')
    photoNumber = String(description='Номер фото')
    photoDate = DateTime(description='Дата фотографии')


# TODO: фотографии должны быть списком фотографий
MOUNTING_FIELDS = OrderedDict([
    ('id', ID(description='Идентификатор задачи')),
    ('mountingRange', Field(MOUNTING_RANGE_ENUM, description='Приоритет монтажа')),
    ('archived', Boolean(description='Архивная задача')),
    ('startMounting', DateTime(description='Дата монтажа')),
    ('endMounting', DateTime(description='Дата демонтажа')),
    ('mountingDone', Boolean(description='Монтаж выполнен')),
    ('unmountingDone', Boolean(description='Демонтаж выполнен')),
    ('photos', List(MountingPhotos, description='Фотографии фотоотчета')),
    ('downloadedEarly', DateTime(description='Выгружено ранее')),
    ('comment', String(description='Комментарий')),
    ('design', Field(MountingDesignField, description='Монтируемый дизайн')),
    ('unmountingDesign', Field(MountingDesignField, description='Демонтируемый дизайн')),
    ('crew', Field(MountingCrewField, description='Экипаж')),
])


FIELDS_RESERVATION = OrderedDict([
    ('statusConnection', Boolean(description='Подключено / Не подключено')),
    ('project', Field(MountingProjectField, description='Проект')),
])


FIELDS_CONSTRUCTION_SIDE = OrderedDict([
    ('constructionSideInfo', Field(ConstructionSideInfo, description='Данные конструкции и стороны конструкции')),
])


MountingField = type(
    'MountingField',
    (ObjectType,),
    OrderedDict([
        ('Meta', type('MountingField.Meta', (), {'description': 'Ремонтно-монтажная задача'})),
        ('commonTaskType', String(description='Тип общей задачи, null если монтажная задача')),
        *MOUNTING_FIELDS.items(),
        *FIELDS_RESERVATION.items(),
        *FIELDS_CONSTRUCTION_SIDE.items(),
        ('isCommonTask', Boolean(description='true - задача является общей, false - задача является монтажем')),
        ('name', String(description='Название монтажной задачи')),
    ])
)
