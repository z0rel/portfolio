import graphene
import base64
from django.db.models import Q, F
from django.contrib.postgres.aggregates import StringAgg
from ptc_deco.xlsx.constructor_report_xlsx import ConstructorXlsxQueryset, Flag, InfoTitle
from .utils.gql2python_name_parser import parse_parameters
from ..utils.auth.decorators import login_or_permissions_required
from ..queries.utils import get_offset_limit
from ..queries.utils.ids import convert_ID_to_id
from ..queries.optim.generate_order_by_class import get_fast_search_param, get_order_by_value
from ... import models as m


field_mapper = {
    'partner_type__title': InfoTitle(
        'Тип контрагента',
        lambda v: v.partner_type.title if v.partner_type else None,
        Flag.func.name,
    ),
    'title': InfoTitle('Контрагент', 'title', Flag.attr.name),
    'brands__title': InfoTitle(
        'Бренд',
        lambda v: ' '.join([b.title for b in v.brands.all()]),
        Flag.func.name
    ),
    'working_sectors__description': InfoTitle(
        'Сектор деятельности',
        lambda v: ' '.join([w.description for w in v.working_sectors.all()]),
        Flag.func.name,
    ),
    'client_type__title': InfoTitle(
        'Тип клиента',
        lambda v: v.client_type.title if v.client_type else None,
        Flag.func.name,
    ),
}


ORDERING_FIELDS = {
    'type': 'partner_type__title',
    'partner': 'title',
    'brand': 'brands_list',
    'sector': 'working_sectors_list',
    'client': 'client_type__title',
    'bin_number': 'bin_number',
}


def init_fast_search_query(kwargs):
    """Создать спецификатор фильтра быстрого поиска"""

    fast_str, fast_int, fast_date = get_fast_search_param(kwargs)

    q = Q()
    if fast_str:
        q |= Q(
                Q(projects_agencies__title__icontains=fast_str) |
                Q(projects_agencies__code__icontains=fast_str) |
                Q(projects_agencies__comment__icontains=fast_str) |
                Q(title__icontains=fast_str) |
                Q(brands__title__icontains=fast_str) |
                Q(partner_type__title__icontains=fast_str) |
                Q(working_sectors__title__icontains=fast_str) |
                Q(client_type__title__icontains=fast_str)
        )
    if fast_date:
        q |= Q(
                Q(projects_agencies__created_at__year=fast_date.year) &
                Q(projects_agencies__created_at__month=fast_date.month) &
                Q(projects_agencies__created_at__day=fast_date.day)
        )

    return q


def db_query(kwargs):
    if 'id' in kwargs:
        kwargs['id'] = convert_ID_to_id(kwargs['id'])

    order_by_value = get_order_by_value(kwargs, ORDERING_FIELDS)

    offset, limit = get_offset_limit(kwargs, None)

    fast_q = init_fast_search_query(kwargs)

    filter_expr = Q(Q(**kwargs) & fast_q)

    partners = m.Partner.objects.filter(
        filter_expr
    ).select_related(
        'client_type',
        'partner_type',
    ).prefetch_related(
        'working_sectors',
        'brands',
    ).annotate(
        brands_list=StringAgg(
            F('brands__title'), delimiter=',', distinct=True, ordering=F('brands__title')
        ),
        working_sectors_list=StringAgg(
            F('working_sectors__description'),
            delimiter=',',
            distinct=True,
            ordering=F('working_sectors__description'),
        ),
    )

    if order_by_value:
        partners = partners.order_by(order_by_value)

    count = partners.count()

    return partners, offset, limit, count


class DownloadPartnersInfo(graphene.Mutation):
    class Arguments:
        query_parameters = graphene.JSONString()
        include_fields = graphene.List(graphene.String)

    file = graphene.String(description='Файл *.xlsx в кодировке Base64')
    ok = graphene.Boolean()

    @login_or_permissions_required(login_required=True, permissions=('api.view_contract', ))
    def mutate(root, info, **input):
        include_fields = input.get('include_fields', None)
        if not include_fields:
            include_fields = field_mapper.values()
        else:
            include_fields = tuple(
                [value for key, value in field_mapper.items() if key in include_fields]
            )

        query_parameters = input.get('query_parameters', None)
        query_parameters = parse_parameters(query_parameters)

        partners, offset, limit, count = db_query(query_parameters)

        if limit:
            partners = partners[offset: offset + limit]
        else:
            partners = partners[offset:]

        constructor = ConstructorXlsxQueryset(partners, 'partners', include_fields)
        constructor.write()

        output = base64.b64encode(constructor.get_virtual_workbook().read()).decode('ascii')
        return DownloadPartnersInfo(file=output, ok=True)
