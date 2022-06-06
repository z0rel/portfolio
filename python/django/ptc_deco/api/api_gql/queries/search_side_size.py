import graphene
from graphene.relay import Node, ConnectionField
from graphene import Connection
from graphene import ID, String, Int
import graphene as g
from ....api import models as m
from .utils import unpack_ids, construct_related_and_only_lists


class SideSizeType(g.ObjectType):
    class Meta:
        interfaces = (Node,)

    title = g.String()  # Наименование стороны
    size = g.String()  # Размер конструкции
    code = g.String()  # Наименование кода стороны


class SideSizeConnection(Connection):
    class Meta:
        node = SideSizeType

    count = Int()

    def resolve_count(root, info):
        return len(root.edges)


class SideSizeQuery(g.ObjectType):
    side_size = ConnectionField(SideSizeConnection)


search_side_size_field = graphene.Field(
    SideSizeQuery,
    description='Размеры сторон конструкций',
    title__icontains=String(description='Наименование типа стороны содержит (регистронезависимо)'),
    title=String(description='Наименование типа стороны'),
    size=String(description='Значение размера'),
    size__icontains=String(description='Значение размера содержит (регистронезависимо)'),
    format__title=String(description='Наименование формата'),
    format__model__underfamily__family__id=ID(description='Идентификатор семейства'),
    format__model__underfamily__family__title=String(description='Наименование семейства'),
    format__model__underfamily__family__title__icontains=String(description='Наименование семейства содержит'),
)


SELECT_RELATED_ARGS_MAPPED = {
    'format__model__underfamily__family__id': [
        ['format__model__underfamily__family'],
        ['format__model__underfamily__family__id'],
    ],
    'format__model__underfamily__family__title': [
        ['format__model__underfamily__family'],
        ['format__model__underfamily__family__title'],
    ],
    'format__model__underfamily__family__title__icontains': [
        ['format__model__underfamily__family'],
        ['format__model__underfamily__family__title'],
    ],
    'format__title': [['format'], ['format__title']],
}

IDS_TO_UNPACK = [
    'format__model__underfamily__family__id',
]


ONLY_FIELDS_SIZE = ['size', 'id']
ONLY_FIELDS_TITLE = ['size', 'id', 'title', 'code']


def resolve_search_side_size(parent, *args, **kwargs):
    unpack_ids(kwargs, IDS_TO_UNPACK)

    only_list, related_list = construct_related_and_only_lists(
        kwargs, ONLY_FIELDS_SIZE, None, SELECT_RELATED_ARGS_MAPPED
    )

    result = m.Side.objects.filter(**kwargs).select_related(*related_list).only(*only_list).order_by('size', 'title')

    dst_result = {}
    for size_row in result:
        if size_row.size:
            dst_result[size_row.size] = size_row.id
    dst = []
    for size, id_val in sorted(dst_result.items()):
        dst.append(SideSizeType(id=id_val, size=size))
    return SideSizeQuery(side_size=dst)


def resolve_search_sides(parent, *args, **kwargs):
    unpack_ids(kwargs, IDS_TO_UNPACK)

    only_list, related_list = construct_related_and_only_lists(
        kwargs, ONLY_FIELDS_TITLE, None, SELECT_RELATED_ARGS_MAPPED
    )

    result = m.Side.objects.filter(**kwargs).select_related(*related_list).only(*only_list).order_by('title')

    dst_result = {}
    for size_row in result:
        if size_row.title:
            dst_result[size_row.title] = size_row

    dst = []
    for size, val in sorted(dst_result.items()):
        dst.append(SideSizeType(id=val.id, size=val.size, title=val.title, code=val.code))
    return SideSizeQuery(side_size=dst)
