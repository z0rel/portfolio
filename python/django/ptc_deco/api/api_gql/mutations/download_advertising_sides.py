import graphene
import base64
from ptc_deco.xlsx.constructor_report_xlsx import ConstructorXlsxQueryset, Flag, InfoTitle
from .utils.gql2python_name_parser import parse_parameters
from ..utils.auth.decorators import login_or_permissions_required
from ..queries.optim.sales.optim_reservations.db_query import db_query
from ..utils.openpyxl import copy_sheet_to_workbook
from ... import models as m


def get_code(v, delimiter='.', default='*'):
    code_parts = []
    if v.construction and v.construction.postcode and v.construction.postcode.title:
        code_parts.append(v.construction.postcode.title)
    else:
        code_parts.append(default)
    if v.construction and v.construction.num_in_district:
        code_parts.append(v.construction.num_in_district)
    else:
        code_parts.append(default)
    if v.advertising_side and v.advertising_side.format and v.advertising_side.format.code:
        code_parts.append(v.advertising_side.side.format.code)
    else:
        code_parts.append(default)
    if v.advertising_side and v.advertising_side.side and v.advertising_side.side.code:
        code_parts.append(v.advertising_side.side.code)
    else:
        code_parts.append(default)
    if v.advertising_side and v.advertising_side.code:
        code_parts.append(v.advertising_side.code)
    else:
        code_parts.append(default)
    return delimiter.join(map(str, code_parts))


field_mapper = {
    'id': InfoTitle(
        'Идентификатор',
        'id',
        Flag.attr.name,
    ),
    'code': InfoTitle(
        'Код',
        lambda v: get_code(v),
        Flag.func.name,
    ),
    'format': InfoTitle(
        'Формат',
        lambda v: v.advertising_side.side.format.title
        if v.advertising_side
        and v.advertising_side.side
        and v.advertising_side.side.format
        else None,
        Flag.func.name,
    ),
    'city': InfoTitle(
        'Город',
        lambda v: v.construction.location.postcode.district.city.title
        if v.construction
        and v.construction.location
        and v.construction.location.postcode
        and v.construction.location.postcode.district
        and v.construction.location.postcode.district.city
        else None,
        Flag.func.name,
    ),
    'district': InfoTitle(
        'Район',
        lambda v: v.construction.location.postcode.district.title
        if v.construction
        and v.construction.location
        and v.construction.location.postcode
        and v.construction.location.postcode.district
        else None,
        Flag.func.name,
    ),
    'marketing_address': InfoTitle(
        'Район',
        lambda v: v.construction.location.marketing_address.address
        if v.construction
        and v.construction.location
        and v.construction.location.marketing_address
        else None,
        Flag.func.name,
    ),
    'side': InfoTitle(
        'Сторона',
        lambda v: v.advertising_side.side.title
        if v.advertising_side
        and v.advertising_side.side
        else None,
        Flag.func.name,
    ),
    'size': InfoTitle(
        'Размер',
        lambda v: v.advertising_side.side.size
        if v.advertising_side
        and v.advertising_side.side
        else None,
        Flag.func.name,
    ),
    'owner': InfoTitle(
        'Владелец',
        lambda v: v.construction.nonrts_owner.title
        if v.construction
        and v.construction.nonrts_owner
        else None,
        Flag.func.name,
    ),
    'package': InfoTitle(
        'Пакет',
        lambda v: v.package.title if v.package else None,
        Flag.func.name,
    ),
    'status_connection': InfoTitle(
        'Пакет',
        lambda v: 'Да' if v.construction and v.construction.status_connection else 'Нет',
        Flag.func.name,
    ),
}


reservations_field_mapper = {
    'construction_side_id': InfoTitle(
        'Идентификатор стороны',
        lambda v: v.construction_side.id if v.construction_side else None,
        Flag.func.name,
    ),
    'start_date': InfoTitle(
        'Начало бронирования',
        lambda v: getattr(v, 'date_from').replace(tzinfo=None) if getattr(v, 'date_from') else None,
        Flag.func.name,
    ),
    'end_date': InfoTitle(
        'Окончание бронирования',
        lambda v: getattr(v, 'date_to').replace(tzinfo=None) if getattr(v, 'date_to') else None,
        Flag.func.name,
    ),
    'project_code': InfoTitle(
        'Код проекта',
        lambda v: v.project.code if v.project else None,
        Flag.func.name,
    ),
    'project_title': InfoTitle(
        'Наименование проекта',
        lambda v: v.project.title if v.project else None,
        Flag.func.name,
    ),
    'client_title': InfoTitle(
        'Наименование клиента',
        lambda v: v.project.client.title
        if v.project
        and v.project.client
        else None,
        Flag.func.name,
    ),
}


class DownloadAdvertisingSidesInfo(graphene.Mutation):
    class Arguments:
        query_parameters = graphene.JSONString()
        include_fields = graphene.List(graphene.String)

    file = graphene.String(description='Файл *.xlsx в кодировке Base64')
    ok = graphene.Boolean()

    @login_or_permissions_required(login_required=True, permissions=('api.view_advertisingside', ))
    def mutate(root, info, **input):
        include_fields = input.get('include_fields', None)
        if not include_fields:
            include_fields = field_mapper.values()
        else:
            include_fields = tuple(
                [value for key, value in field_mapper.items() if key in include_fields]
            )
        reservations_include_fields = reservations_field_mapper.values()

        query_parameters = input.get('query_parameters', None)
        query_parameters = parse_parameters(query_parameters)

        construction_sides, offset, limit, count = db_query(query_parameters)

        reservations_ids = set()
        for cs in construction_sides:
            for reservation in cs.reservation.all():
                reservations_ids.add(reservation.id)

        reservations = m.Reservation.objects.filter(
            id__in=reservations_ids
        ).select_related(
            'project',
            'project__client',
            'construction_side',
        ).order_by(
            'construction_side',
            'date_from',
            'date_to',
            'reservation_type',
        )

        reservations_constructor = ConstructorXlsxQueryset(reservations, 'reservations', reservations_include_fields)
        reservations_constructor.write()

        constructor = ConstructorXlsxQueryset(construction_sides, 'advertising_sides', include_fields)
        constructor.write()

        copy_sheet_to_workbook(constructor.wb, reservations_constructor.sheet, 'Брони')

        output = base64.b64encode(constructor.get_virtual_workbook().read()).decode('ascii')
        return DownloadAdvertisingSidesInfo(file=output, ok=True)
