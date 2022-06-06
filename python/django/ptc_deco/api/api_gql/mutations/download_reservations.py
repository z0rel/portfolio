import graphene
import base64
from django.db.models import Q
from ptc_deco.xlsx.constructor_report_xlsx import ConstructorXlsxQueryset, Flag, InfoTitle
from .utils.gql2python_name_parser import parse_parameters
from ..queries.utils import unpack_ids
from ..utils.auth.decorators import login_or_permissions_required
from ..queries.utils import get_offset_limit
from ..queries.optim.generate_order_by_class import get_fast_search_param, get_order_by_value
from ... import models as m


def get_code(v, delimiter='.', default='*'):
    code_parts = []
    if v.construction_side:
        if v.construction_side.construction:
            if v.construction_side.construction.postcode:
                code_parts.append(
                    v.construction_side.construction.postcode.title
                    if v.construction_side.construction.postcode.title
                    else default
                )
            else:
                code_parts.append(default)
            code_parts.append(
                v.construction_side.construction.num_in_district
                if v.construction_side.construction.num_in_district
                else default
            )
        else:
            code_parts += [default for i in range(2)]
        if v.construction_side.advertising_side:
            if v.construction_side.advertising_side.side and v.construction_side.advertising_side.side.format:
                code_parts.append(
                    v.advertising_side.side.format.code
                    if v.advertising_side.side.format.code
                    else default
                )
            else:
                code_parts.append(default)
            if v.construction_side.advertising_side.side:
                code_parts.append(
                    v.construction_side.advertising_side.side.code
                    if v.construction_side.advertising_side.side.code
                    else default
                )
            else:
                code_parts.append(default)
            code_parts.append(
                v.construction_side.advertising_side.code
                if v.construction_side.advertising_side.code
                else default
            )
        else:
            code_parts += [default for i in range(3)]
    else:
        code_parts = ['*' * 5]
    return delimiter.join(map(str, code_parts))


field_mapper = {
    'code': InfoTitle(
        'Код стороны',
        lambda v: get_code(v),
        Flag.func.name,
    ),
    'city': InfoTitle(
        'Город',
        lambda v: v.construction_side.construction.location.postcode.district.city.title
        if v.construction_side
        and v.construction_side.construction
        and v.construction_side.construction.location
        and v.construction_side.construction.location.postcode
        and v.construction_side.construction.location.postcode.district
        and v.construction_side.construction.location.postcode.district.city
        and v.construction_side.construction.location.postcode.district.city.title
        else None,
        Flag.func.name,
    ),
    'address': InfoTitle(
        'Адрес',
        lambda v: v.construction_side.construction.location.marketing_address.address
        if v.construction_side
        and v.construction_side.construction
        and v.construction_side.construction.location
        and v.construction_side.construction.location.marketing_address
        and v.construction_side.construction.location.marketing_address.address
        else None,
        Flag.func.name,
    ),
    'format': InfoTitle(
        'Формат',
        lambda v: v.construction_side.advertising_side.side.format.title
        if v.construction_side
        and v.construction_side.advertising_side
        and v.construction_side.advertising_side.side
        and v.construction_side.advertising_side.side.format
        and v.construction_side.advertising_side.side.format.title
        else None,
        Flag.func.name,
    ),
    'creation_date': InfoTitle(
        'Дата создания',
        lambda v: getattr(v, 'creation_date').replace(tzinfo=None) if getattr(v, 'creation_date') else None,
        Flag.func.name,
    ),
    'date_form': InfoTitle(
        'Дата начала',
        lambda v: getattr(v, 'date_from').replace(tzinfo=None) if getattr(v, 'date_from') else None,
        Flag.func.name,
    ),
    'date_to': InfoTitle(
        'Дата окончания',
        lambda v: getattr(v, 'date_to').replace(tzinfo=None) if getattr(v, 'date_to') else None,
        Flag.func.name,
    ),
    'branding': InfoTitle(
        'Брендирование',
        lambda v: 'Да' if v.branding else 'Нет',
        Flag.func.name,
    )
}


ORDERING_FIELDS = {
    'postcode_title': 'construction_side__construction__location__postcode__title',
    'num_in_district': 'construction_side__construction__num_in_district',
    'format_code': 'construction_side__advertising_side__side__format__code',
    'side_code': 'construction_side__advertising_side__side__code',
    'adv_side_code': 'construction_side__advertising_side__code',
    'reservation_city': 'construction_side__construction__location__postcode__district__city__title',
    'reservation_address': 'construction_side__construction__location__marketing_address__address',
    'reservation_format': 'construction_side__advertising_side__side__format__title',
    'reservation_side': 'construction_side__advertising_side__side__title',
    'reservation_creation_date': 'creation_date',
    'reservation_start_date': 'date_from',
    'reservation_expiration_date': 'date_to',
    'reservation_status': 'reservation_type__title',
    'reservation_branding': 'branding',
    'reservation_lighting': 'construction_side__construction__status_connection',
    'reservation_package': 'construction_side__package__title',

}


IDS_TO_UNPACK = [
    'id',
    'project_id',
    'appendix__id',
    'construction_side_advertising_side__side__format__model__underfamily__family__id',
    'construction_side__construction__location__postcode__district__id',
    'construction_side__construction__location__postcode__district__city__id',
]


def init_fast_search_query(kwargs):
    """Создать спецификатор фильтра быстрого поиска"""

    fast_str, fast_int, fast_date = get_fast_search_param(kwargs)

    q = Q()
    if fast_str:
        q |= Q(
                Q(branding__icontains=fast_str) |
                Q(construction_side__construction__location__postcode__district__city__title__icontains=fast_str) |
                Q(construction_side__construction__location__marketing_address__address__icontains=fast_str) |
                Q(construction_side__construction__location__postcode__title__icontains=fast_str) |
                Q(construction_side__advertising_side__side__format__title__icontains=fast_str) |
                Q(construction_side__advertising_side__side__format__code__icontains=fast_str) |
                Q(construction_side__advertising_side__side__code__icontains=fast_str) |
                Q(construction_side__advertising_side__code__icontais=fast_str)
            )
    if fast_int:
        q |= Q(construction_side__construction__num_in_district=fast_int)
    if fast_date:
        q |= Q(
            Q(
                Q(date_from__year=fast_date.year) &
                Q(date_from__month=fast_date.month) &
                Q(date_from__day=fast_date.day)
            ) |
            Q(
                Q(date_to__year=fast_date.year) &
                Q(date_to__month=fast_date.month) &
                Q(date_to__day=fast_date.day)
            ) |
            Q(
                Q(creation_date__year=fast_date.year) &
                Q(creation_date__month=fast_date.month) &
                Q(creation_date__day=fast_date.day)
            )
        )

    return q


def db_query(kwargs):
    unpack_ids(kwargs, IDS_TO_UNPACK)

    order_by_value = get_order_by_value(kwargs, ORDERING_FIELDS)

    offset, limit = get_offset_limit(kwargs, None)

    fast_q = init_fast_search_query(kwargs)

    filter_expr = Q(Q(**kwargs) & fast_q)

    reservations = m.Reservation.objects.filter(
        filter_expr
    ).select_related(
        'construction_side__construction__location__postcode__district__city',
        'construction_side__construction__location__marketing_address',
        'construction_side__advertising_side__side__format',
    )

    if order_by_value:
        reservations = reservations.order_by(order_by_value)

    count = reservations.count()

    return reservations, offset, limit, count


class DownloadReservationsInfo(graphene.Mutation):
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

        reservations, offset, limit, count = db_query(query_parameters)

        if limit:
            reservations = reservations[offset: offset + limit]
        else:
            reservations = reservations[offset:]

        constructor = ConstructorXlsxQueryset(reservations, 'reservations', include_fields)
        constructor.write()

        output = base64.b64encode(constructor.get_virtual_workbook().read()).decode('ascii')
        return DownloadReservationsInfo(file=output, ok=True)
