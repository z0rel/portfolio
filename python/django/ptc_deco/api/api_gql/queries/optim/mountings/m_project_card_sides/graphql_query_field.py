from graphene import ObjectType, Field, ID, List, Date, DateTime, Int, Boolean

from ...generate_order_by_class import graphql__order_by__offset__limit__mixin, graphql__fastsearch__mixin
from ....utils import PageInfoApi, ContentFieldConnection

from .graphql_answer_main import AnswerMountingProjectCardField
from .order_by import ORDERING_FIELDS


class MountingProjectCardFieldConnection(ObjectType):
    class Meta:
        description = 'Бронирование с данными о монтаже'

    content = List(AnswerMountingProjectCardField, description='Бронирование с данными о монтаже')
    pageInfo = Field(PageInfoApi, description='Данные пагинации')


MOUNTING_PROJECT_CARD_FILTER_FIELDS = {
    'project_id': ID(description='Идентификатор проекта', required=True),
    'start_mounting__contains': Date(description='Монтаж на заданную дату'),
    'start_mounting__gt': DateTime(description='Монтаж после заданной даты'),
    'start_mounting__gte': DateTime(description='Монтаж на заданную дату или позднее'),
    'start_mounting__lt': DateTime(description='Монтаж до заданной даты'),
    'start_mounting__lte': DateTime(description='Монтаж на заданную дату или ранее'),
    'end_mounting__contains': Date(description='Демонтаж на заданную дату'),
    'end_mounting__gt': DateTime(description='Демонтаж после заданной даты'),
    'end_mounting__gte': DateTime(description='Демонтаж на заданную дату или позднее'),
    'end_mounting__lt': DateTime(description='Демонтаж до заданной даты'),
    'end_mounting__lte': DateTime(description='Демонтаж на заданную дату или ранее'),
    'mounting_tasks_offset': Int(description='Монтажные задачи - смещение в списке'),
    'mounting_tasks_limit': Int(description='Монтажные задачи - максимальное число элементов в списке'),
    'additional_photo_day_offset': Int(description='Дополнительный дневной фотоотчет - смещение в списке'),
    'additional_photo_day_limit': Int(
        description='Дополнительный дневной фотоотчет - максимальное число элементов в списке'
    ),
    'additional_photo_night_offset': Int(description='Дополнительный ночной фотоотчет - смещение в списке'),
    'additional_photo_night_limit': Int(
        description='Дополнительный ночной фотоотчет - максимальное число элементов в списке'
    ),
    **graphql__order_by__offset__limit__mixin('MountingProjectCardOrderBy', ORDERING_FIELDS),
    **graphql__fastsearch__mixin(),
}


RESERVATION_PROJECT_CARD_FILTER_FIELDS = {
    'reservation__date_from': DateTime(description='Начало бронирования на заданную дату'),
    'reservation__date_from__contains': Date(description='Начало бронирования содержит'),
    'reservation__date_from__gt': DateTime(description='Начало бронирования после заданной даты'),
    'reservation__date_from__gte': DateTime(description='Начало бронирования на заданную дату или позднее'),
    'reservation__date_from__lt': DateTime(description='Начало бронирования ранее'),
    'reservation__date_from__lte': DateTime(description='Начало бронирования на заданную дату или ранее'),

    'reservation__date_to': DateTime(description='Окончание бронирования бронирования на заданную дату'),
    'reservation__date_to__contains': Date(description='Окончание бронирования бронирования содержит'),
    'reservation__date_to__gt': DateTime(description='Окончание бронирования бронирования после заданной даты'),
    'reservation__date_to__gte': DateTime(description='Окончание бронирования бронирования на заданную дату или позднее'),
    'reservation__date_to__lt': DateTime(description='Окончание бронирования бронирования ранее'),
    'reservation__date_to__lte': DateTime(description='Окончание бронирования бронирования на заданную дату или ранее'),

    'reservation__branding': Boolean(description='Наличие брендинга для бронирования'),
}

field = Field(
    MountingProjectCardFieldConnection,
    **MOUNTING_PROJECT_CARD_FILTER_FIELDS,
    **RESERVATION_PROJECT_CARD_FILTER_FIELDS,
    description='Проект монтажа (graphql-структура)'
)


field_proto = Field(
    ContentFieldConnection, **MOUNTING_PROJECT_CARD_FILTER_FIELDS, description='Проект монтажа (protobuf-сообщение)'
)
