from graphene import ObjectType, Boolean, Field, String, DateTime, ID, Int, List, Date

from ...generate_order_by_class import graphql__order_by__offset__limit__mixin, graphql__fastsearch__mixin
from ....utils import ContentJSONFieldConnection, PageInfoApi

from .graphql_answer_main import MountingField
from .order_by import ORDERING_FIELDS


class MountingFieldConnection(ObjectType):
    class Meta:
        description = 'Ремонтно-монтажные задачи'

    content = List(MountingField, description='Ремонтно-монтажные задачи')
    pageInfo = Field(PageInfoApi, description='Данные пагинации')


MOUNTING_COMMON_FILTER_FIELDS = {
    'city_id': ID(description='Идентификатор города'),
    'city_title': String(description='Название города (точное совпадение)'),
    'city_title__icontains': String(description='Название города содержит (регистронезависимо)'),

    'district_id': ID(description='Идентификатор района'),
    'postcode_id': ID(description='Идентификатор почтового кода'),
    'project_id': ID(description='Идентификатор проекта'),
    'format_id': ID(description='Идентификатор формата'),
    'format_title': String(description='Название формата (точное совпадение)'),
    'mounting_task__title__icontains': String(description='Тип задачи содержит (регистронезависимо)'),

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

    'archived': Boolean(description='Выбрать архивные задачи (true) либо текущие (false)'),
    'common_task': Boolean(description='Выбрать общие задачи (true) либо монтажи (false)'),
    **graphql__order_by__offset__limit__mixin('MountingOrderBy', ORDERING_FIELDS),
    **graphql__fastsearch__mixin()
}


mountings_debug_field = Field(
    MountingFieldConnection,
    **MOUNTING_COMMON_FILTER_FIELDS,
    description='Монтажи (graphql-структура)'
)


mountings_field = Field(
    ContentJSONFieldConnection,
    **MOUNTING_COMMON_FILTER_FIELDS,
    description='Монтажи (json-строка)'
)
