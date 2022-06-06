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


field_mapper = {
    'code': InfoTitle('Номер приложения', 'code', Flag.attr.name),
    # summa
    'created_date': InfoTitle(
        'Дата создания',
        lambda v: v.created_date.replace(tzinfo=None) if v.created_date else None,
        Flag.func.name,
    ),
    'contract__partner_title': InfoTitle(
        'Контрагент',
        lambda v: v.contract.partner.title if v.contract and v.contract.partner else None,
        Flag.func.name,
    ),
    'project_title': InfoTitle(
        'Проект',
        lambda v: v.project.title if v.project else None,
        Flag.func.name,
    ),
    'period_start_date': InfoTitle(
        'Дата заключения',
        lambda v: getattr(v, 'period_start_date').replace(tzinfo=None) if getattr(v, 'period_start_date') else None,
        Flag.func.name,
    ),
    'period_end_date': InfoTitle(
        'Дата окончания',
        lambda v: getattr(v, 'period_end_date').replace(tzinfo=None) if getattr(v, 'period_end_date') else None,
        Flag.func.name,
    ),
    'payment_date': InfoTitle(
        'Срок оплаты',
        lambda v: v.payment_date.replace(tzinfo=None) if v.payment_date else None,
        Flag.func.name,
    ),
}


ORDERING_FIELDS = {
    'code': 'code',
    'partner': 'contract__partner__title',
    'project': 'project__title',
    'date_start': 'period_start_date',
    'date_end': 'period_end_date',
    'contract__code': 'contract__code',
    'brand': 'project__brand__title',
    'sector': 'project__brand__working_sector__description',
    'create': 'created_date',
    'creator': 'creator__last_name',
    'creator_first_name': 'creator__first_name',
}


IDS_TO_UNPACK = [
    'id',
    'creator',
    'contract_id',
    'contract__partner_id',
    'project__client_id',
    'project__back_office_manager_id',
    'sales_manager_id',
    'project_id',
    'project__id',
    'project__brand_id',
]


def init_fast_search_query(kwargs):
    """Создать спецификатор фильтра быстрого поиска"""

    fast_str, fast_int, fast_date = get_fast_search_param(kwargs)

    q = Q()
    if fast_str:
        q |= Q(
                Q(code__icontains=fast_str) |
                Q(project__title__icontains=fast_str) |
                Q(contract__partner__title__icontains=fast_str)
                # добавить поиск по сумме
                # если она вычисляется, то сделать аннотацию в db_query
                # и после фильтровать тут по имени созданного поля
            )
    if fast_date:
        q |= Q(
            Q(
                Q(payment_date__year=fast_date.year) &
                Q(payment_date__month=fast_date.month) &
                Q(payment_date__day=fast_date.day)
            ) |
            Q(
                Q(created_date__year=fast_date.year) &
                Q(created_date__month=fast_date.month) &
                Q(created_date__day=fast_date.day)
            ) |
            Q(
                Q(period_start_date__year=fast_date.year) &
                Q(period_start_date__month=fast_date.month) &
                Q(period_start_date__day=fast_date.day)
            ) |
            Q(
                Q(period_end_date__year=fast_date.year) &
                Q(period_end_date__month=fast_date.month) &
                Q(period_end_date__day=fast_date.day)
            )
        )

    return q


def db_query(kwargs):
    unpack_ids(kwargs, IDS_TO_UNPACK)

    order_by_value = get_order_by_value(kwargs, ORDERING_FIELDS)

    offset, limit = get_offset_limit(kwargs, None)

    fast_q = init_fast_search_query(kwargs)

    filter_expr = Q(Q(**kwargs) & fast_q)

    appendices = m.Appendix.objects.filter(
        filter_expr
    ).select_related(
        'project',
        'creator',
        'contract',
        'contract__partner',
        'project',
        'sales_manager',
    )

    if order_by_value:
        appendices = appendices.order_by(order_by_value)

    count = appendices.count()

    return appendices, offset, limit, count


class DownloadAppendicesInfo(graphene.Mutation):
    class Arguments:
        query_parameters = graphene.JSONString()
        include_fields = graphene.List(graphene.String)

    file = graphene.String(description='Файл *.xlsx в кодировке Base64')
    ok = graphene.Boolean()

    @login_or_permissions_required(login_required=True, permissions=('api.view_appendix', ))
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

        appendices, offset, limit, count = db_query(query_parameters)

        if limit:
            appendices = appendices[offset: offset + limit]
        else:
            appendices = appendices[offset:]

        constructor = ConstructorXlsxQueryset(appendices, 'appendices', include_fields)
        constructor.write()

        output = base64.b64encode(constructor.get_virtual_workbook().read()).decode('ascii')
        return DownloadAppendicesInfo(file=output, ok=True)
