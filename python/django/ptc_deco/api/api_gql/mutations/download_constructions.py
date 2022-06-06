import graphene
import base64
from django.db.models import Q, F, Count
from django.contrib.postgres.aggregates import StringAgg, ArrayAgg
from ptc_deco.xlsx.constructor_report_xlsx import ConstructorXlsxQueryset, Flag, InfoTitle
from .utils.gql2python_name_parser import parse_parameters
from ..utils.auth.decorators import login_or_permissions_required
from ..queries.utils import get_offset_limit
from ..queries.utils.ids import convert_ID_to_id
from ..queries.optim.generate_order_by_class import get_fast_search_param, get_order_by_value
from ... import models as m


field_mapper = {
    'code': InfoTitle(
        'Код конструкции',
        lambda v: v.location.postcode.title + '.' + str(v.num_in_district)
        if v.location and v.location.postcode and v.num_in_district
        else None,
        Flag.func.name,
    ),
    'formats__format__title': InfoTitle(
        'Формат',
        lambda v: ' '.join([f.format.title for f in v.formats.all()]),
        Flag.func.name,
    ),
    'location__postcode__district__city__title': InfoTitle(
        'Город',
        lambda v: v.location.postcode.district.city.title
        if v.location and v.location.postcode and v.location.postcode.district and v.location.postcode.district.city
        else None,
        Flag.func.name,
    ),
    'location__marketing_address__address': InfoTitle(
        'Адрес',
        lambda v: v.location.marketing_address.address
        if v.location and v.location.marketing_address
        else None,
        Flag.func.name,
    ),
    'status_connection': InfoTitle('Статус', 'status_connection', Flag.attr.name),
    'created_at': InfoTitle(
        'Дата начала',
        lambda v: getattr(v, 'created_at').replace(tzinfo=None) if getattr(v, 'created_at') else None,
        Flag.func.name,
    ),
}

ORDERING_FIELDS = {
    'formats_codes_list': 'formats_codes_list',
    'num_in_district': 'num_in_district',
    'city': 'postcode__district__city__title',
    'district': 'postcode__district__title',
    'post': 'postcode__title',
    'adress_m': 'marketing_address__address',
    'adress_j': 'legal_address__address',
    'inv_oto': 'tech_invent_number',
    'inv_buh': 'buh_invent_number',
    'model': 'model__title',
    'underfamily': 'model__underfamily__title',
    'phone': 'tech_phone_construction',
    'coords': 'coordinates',
    'fire': 'status_connection',
    'status_availability': 'status_availability',
    'format': 'formats_list',
    'formats_count': 'formats_count',
}


IDS_TO_UNPACK = [
    'id',
    'crew',
]


def init_fast_search_query(kwargs):
    """Создать спецификатор фильтра быстрого поиска"""

    fast_str, fast_int, fast_date = get_fast_search_param(kwargs)

    q = Q()
    if fast_str:
        q |= Q(
                Q(postcode__district__city__title__icontains=fast_str) |
                Q(postcode__title__icontains=fast_str) |
                Q(marketing_address__address__icontains=fast_str) |
                Q(legal_address__address__icontains=fast_str) |
                Q(model__title__icontains=fast_str) |
                Q(coordinates__icontains=fast_str) |
                Q(formats__format__title__icontains=fast_str) |
                Q(status_connection__icontains=fast_str)
            )
    if fast_int:
        q |= Q(row_idx=fast_int)

    return q


def db_query(kwargs):
    if 'id' in kwargs:
        kwargs['id'] = convert_ID_to_id(kwargs['id'])

    order_by_value = get_order_by_value(kwargs, ORDERING_FIELDS)

    offset, limit = get_offset_limit(kwargs, None)

    fast_q = init_fast_search_query(kwargs)

    filter_expr = Q(Q(**kwargs) & fast_q)

    contracts = m.Construction.objects.filter(
        filter_expr
    ).select_related(
        'location__postcode__district__city',
        'location__marketing_address',
    ).prefetch_related(
        'formats__format',
    ).annotate(
        formats_arr=ArrayAgg(
            F('formats__format__title'),
            delimiter=', ',
            ordering=F('formats__format__title'),
            filter=Q(formats__count__gt=0),
        ),
        formats_list=StringAgg(
            F('formats__format__title'),
            delimiter=', ',
            ordering=F('formats__format__title'),
            filter=Q(formats__count__gt=0),
        ),
        formats_codes_list=StringAgg(
            F('formats__format__code'),
            delimiter=',',
            ordering=F('formats__format__title'),
            filter=Q(formats__count__gt=0),
        ),
        formats_count=Count(F('formats'), filter=Q(formats__count__gt=0)),
    )

    if order_by_value:
        contracts = contracts.order_by(order_by_value)

    count = contracts.count()

    return contracts, offset, limit, count


class DownloadConstructionsInfo(graphene.Mutation):
    class Arguments:
        query_parameters = graphene.JSONString()
        include_fields = graphene.List(graphene.String)

    file = graphene.String(description='Файл *.xlsx в кодировке Base64')
    ok = graphene.Boolean()

    @login_or_permissions_required(login_required=True, permissions=('api.view_construction', ))
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

        constructions, offset, limit, count = db_query(query_parameters)

        if limit:
            constructions = constructions[offset: offset + limit]
        else:
            constructions = constructions[offset:]

        constructor = ConstructorXlsxQueryset(constructions, 'constructions', include_fields)
        constructor.write()

        output = base64.b64encode(constructor.get_virtual_workbook().read()).decode('ascii')
        return DownloadConstructionsInfo(file=output, ok=True)
