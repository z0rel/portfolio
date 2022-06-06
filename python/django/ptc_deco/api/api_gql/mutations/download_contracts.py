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
    'code': InfoTitle('Код договора', 'code', Flag.attr.name),
    'partner__title': InfoTitle(
        'Контрагент',
        lambda v: v.partner.title,
        Flag.func.name
    ),
    'project__title': InfoTitle(
        'Проект',
        lambda v: ' '.join([a.project.title for a in v.contract_appendices.all()]),
        Flag.func.name),
    'date_start': InfoTitle('Дата начала', 'start', Flag.attr.name),
    'date_end': InfoTitle('Дата окончания', 'registration_date', Flag.attr.name),
}


ORDERING_FIELDS = {
    'code': 'code',
    'partner': 'partner__title',
    'project': 'contract_appendices__project__title',
    'date_start': 'registration_date',
    'date_end': 'end',
}


def init_fast_search_query(kwargs):
    """Создать спецификатор фильтра быстрого поиска"""

    fast_str, fast_int, fast_date = get_fast_search_param(kwargs)

    q = Q()
    if fast_str:
        q |= Q(
                Q(code__icontains=fast_str) |
                Q(contract_appendices__project__title__icontains=fast_str) |
                Q(partner__title__icontains=fast_str)
            )
    if fast_date:
        q |= Q(
            Q(
                Q(start__year=fast_date.year) &
                Q(start__month=fast_date.month) &
                Q(start__day=fast_date.day)
            ) |
            Q(
                Q(end__year=fast_date.year) &
                Q(end__month=fast_date.month) &
                Q(end__day=fast_date.day)
            )
        )

    return q


def db_query(kwargs):
    if 'id' in kwargs:
        kwargs['id'] = convert_ID_to_id(kwargs['id'])

    order_by_value = get_order_by_value(kwargs, ORDERING_FIELDS)

    offset, limit = get_offset_limit(kwargs, None)

    fast_q = init_fast_search_query(kwargs)

    filter_expr = Q(Q(**kwargs) & fast_q)

    contracts = m.Contract.objects.filter(
        filter_expr
    ).select_related(
        'partner',
    ).prefetch_related(
        'contract_appendices__project',
    ).annotate(
        project_titles_list=StringAgg(
            F('contract_appendices__project__title'),
            delimiter=',',
            distinct=True,
            ordering=F('contract_appendices__project__title'),
        )
    )

    if order_by_value:
        contracts = contracts.order_by(order_by_value)

    count = contracts.count()

    return contracts, offset, limit, count


class DownloadConstractsInfo(graphene.Mutation):
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

        contracts, offset, limit, count = db_query(query_parameters)

        if limit:
            contracts = contracts[offset: offset + limit]
        else:
            contracts = contracts[offset:]

        constructor = ConstructorXlsxQueryset(contracts, 'contracts', include_fields)
        constructor.write()

        output = base64.b64encode(constructor.get_virtual_workbook().read()).decode('ascii')
        return DownloadConstractsInfo(file=output, ok=True)
