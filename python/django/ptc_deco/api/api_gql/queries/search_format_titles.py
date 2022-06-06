import graphene
from django.db.models import Prefetch
from graphene.relay import Node, ConnectionField
from graphene import Connection, Int
from graphene import ID, String, Boolean, DateTime, Field, Int
import graphene as g
from ....api import models as m
from .utils import unpack_ids, construct_related_and_only_lists


class FormatTitleType(g.ObjectType):
    class Meta:
        interfaces = (Node,)

    title = g.String()  # Наименование формата
    code = g.String()  # Наименование кода формата


class FormatTitleConnection(Connection):
    class Meta:
        node = FormatTitleType

    count = Int()

    def resolve_count(root, info):
        return len(root.edges)


class FormatTitleQuery(g.ObjectType):
    format_titles = ConnectionField(FormatTitleConnection)


search_format_titles_field = graphene.Field(
    FormatTitleQuery,
    description='Уникальные наименования форматов',
    title__icontains=String(description='Наименование формата содержит (регистронезависимо)'),
    title=String(description='Наименование формата'),
    code=String(description='Код формата'),
    code__icontains=String(description='Код формата содержит (регистронезависимо)'),
    model__underfamily__family__id=ID(description='Идентификатор семейства'),
    model__underfamily__family__title=String(description='Наименование семейства'),
    model__underfamily__family__title__icontains=String(description='Наименование семейства содержит'),
    side_title=String(description='Наименование стороны'),
)


SELECT_RELATED_ARGS_MAPPED = {
    'model__underfamily__family__id': [['model__underfamily__family'], ['model__underfamily__family__id']],
    'model__underfamily__family__title': [['model__underfamily__family'], ['model__underfamily__family__title']],
    'model__underfamily__family__title__icontains': [
        ['model__underfamily__family'],
        ['model__underfamily__family__title'],
    ],
}


IDS_TO_UNPACK = [
    'model__underfamily__family__id',
]


ONLY_FIELDS = ['title', 'code', 'id']


def resolve_search_formats(parent, *args, **kwargs):
    unpack_ids(kwargs, IDS_TO_UNPACK)
    prefetch_side_title = None
    print(kwargs)
    if 'side_title' in kwargs:
        prefetch_side_title = kwargs.pop('side_title')

    only_list, related_list = construct_related_and_only_lists(kwargs, ONLY_FIELDS, None, SELECT_RELATED_ARGS_MAPPED)
    pf = []
    if prefetch_side_title:
        pf = [Prefetch('sides', queryset=m.Side.objects.filter(title=prefetch_side_title).only('id', 'title'))]

    result = (
        m.Format.objects.filter(**kwargs)
        .select_related(*related_list)
        .prefetch_related(*pf)
        .only(*only_list)
        .order_by('title')
    )

    dst_result = {}
    size_row: m.Format
    if not pf:
        for size_row in result:
            if size_row.title:
                dst_result[size_row.title] = size_row
    else:
        for size_row in result:
            if size_row.title and size_row.sides.exists():
                dst_result[size_row.title] = size_row

    dst = []
    for size, val in sorted(dst_result.items()):
        dst.append(FormatTitleType(id=val.id, title=val.title, code=val.code))
    return FormatTitleQuery(format_titles=dst)
