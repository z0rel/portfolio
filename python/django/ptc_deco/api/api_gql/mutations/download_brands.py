import graphene
import base64
from django.db.models import Q, F
from django.contrib.postgres.aggregates import StringAgg
from ptc_deco.xlsx.constructor_report_xlsx import ConstructorXlsxQueryset, Flag, InfoTitle
from .utils.gql2python_name_parser import parse_parameters
from ..utils.auth.decorators import login_or_permissions_required
from ..queries.utils import get_offset_limit
from ..queries.utils.ids import unpack_ids
from ..queries.optim.generate_order_by_class import get_fast_search_param, get_order_by_value
from ... import models as m


field_mapper = {
    'title': InfoTitle('Наименование', 'title', Flag.attr.name),
    'working_sector__title': InfoTitle(
        'Наименование сектора деятельности',
        lambda v: v.working_sector.title if v.working_sector else None,
        Flag.func.name
    ),
    'partners__title': InfoTitle(
        'Наименование партнера',
        lambda v: ' '.join([p.title for p in v.partners.all()]),
        Flag.func.name,
    ),
}


IDS_TO_UNPACK = [
    'id',
]


ORDERING_FIELDS = {
    'brand': 'title',
    'partner': 'partners_list',
    'working_sector': 'working_sector__description',
}


def init_fast_search_query(kwargs):
    """Создать спецификатор фильтра быстрого поиска"""

    fast_str, fast_int, fast_date = get_fast_search_param(kwargs)

    q = Q()
    if fast_str:
        q |= Q(
                Q(title__icontains=fast_str) |
                Q(working_sector__title__icontains=fast_str) |
                Q(partners__title__icontains=fast_str)
            )

    return q


def db_query(kwargs):

    unpack_ids(kwargs, IDS_TO_UNPACK)

    order_by_value = get_order_by_value(kwargs, ORDERING_FIELDS)

    offset, limit = get_offset_limit(kwargs, None)

    fast_q = init_fast_search_query(kwargs)

    filter_expr = Q(Q(**kwargs) & fast_q)

    brands = m.Brand.objects.filter(
        filter_expr
    ).select_related(
        'working_sector',
    ).prefetch_related(
        'partners',
    ).annotate(
        partners_list=StringAgg(
            F('partners__title'),
            delimiter=',',
            distinct=True,
            ordering=F('partners__title'),
        )
    )

    if order_by_value:
        brands = brands.order_by(order_by_value)

    count = brands.count()

    return brands, offset, limit, count


class DownloadBrandsInfo(graphene.Mutation):
    class Arguments:
        query_parameters = graphene.JSONString()
        include_fields = graphene.List(graphene.String)

    file = graphene.String(description='Файл *.xlsx в кодировке Base64')
    ok = graphene.Boolean()

    @login_or_permissions_required(login_required=True, permissions=('api.view_brand', ))
    def mutate(root, info, **input):
        include_fields = input.get('include_fields', None)
        if not include_fields:
            include_fields = field_mapper.values()
        else:
            include_fields = tuple([value for key, value in field_mapper.items() if key in include_fields])

        query_parameters = input.get('query_parameters', None)
        query_parameters = parse_parameters(query_parameters)

        brands, offset, limit, count = db_query(query_parameters)

        if limit:
            brands = brands[offset: offset + limit]
        else:
            brands = brands[offset:]

        constructor = ConstructorXlsxQueryset(brands, 'brands', include_fields)
        constructor.write()

        output = base64.b64encode(constructor.get_virtual_workbook().read()).decode('ascii')
        return DownloadBrandsInfo(file=output, ok=True)
