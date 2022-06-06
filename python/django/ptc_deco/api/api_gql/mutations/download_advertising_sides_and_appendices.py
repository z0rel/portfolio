import graphene
import base64
from ptc_deco.xlsx.constructor_report_xlsx import ConstructorXlsxQueryset, Flag, InfoTitle
from .utils.gql2python_name_parser import parse_parameters
from ..utils.auth.decorators import login_or_permissions_required
from ..utils.openpyxl import copy_sheet_to_workbook
from ..queries.optim.sales.optim_reservations.db_query import db_query
from ... import models as m

adv_side_field_mapper = {
    'advertising_side_title': InfoTitle('Название рекламной тороны', 'advertising_side_title', Flag.attr.name),
    'availability_side': InfoTitle(
        'Статус доступности',
        lambda v: 'Доступно' if getattr(v, 'availability_side') else 'Недоступно',
        Flag.func.name
    ),
    'is_archive': InfoTitle('В архиве', lambda v: '+' if getattr(v, 'is_archive') else '-', Flag.func.name),
    'construction_id': InfoTitle('Конструкция', 'construction_id', Flag.attr.name),
    'purpose_side_id': InfoTitle('Назначение стороны', 'purpose_side_id', Flag.attr.name),
    'package_title': InfoTitle('Пакет', 'package_title', Flag.attr.name),
    'crew_id': InfoTitle('Идентиификатор экипажа', 'crew_id', Flag.attr.name),
    'sale_constraint': InfoTitle(
        'Ограничение стороны конструкции по продажам',
        lambda v: '+' if getattr(v, 'sale_constraint') else '-',
        Flag.func.name,
    ),
    'created_at': InfoTitle(
        'Дата создания',
        lambda v: getattr(v, 'created_at').replace(tzinfo=None) if getattr(v, 'created_at') else None,
        Flag.func.name,
    ),
    'updated_at': InfoTitle(
        'Дата обновления',
        lambda v: getattr(v, 'updated_at').replace(tzinfo=None) if getattr(v, 'updated_at') else None,
        Flag.func.name,
    ),
}

appendices_field_mapper = {
    'code': InfoTitle('Код приложения', 'code', Flag.attr.name),
    'num_in_month': InfoTitle('Номер приложения в месяце', 'num_in_month', Flag.attr.name),
    'created_date': InfoTitle('Дата создания приложения', 'created_date', Flag.attr.name),
    'return_status': InfoTitle(
        'Статус возврата',
        lambda v: '+' if getattr(v, 'return_status') else '-',
        Flag.func.name
    ),
    'period_start_date': InfoTitle(
        'Период приложения - дата начала размещения',
        lambda v: getattr(v, 'period_start_date').replace(tzinfo=None) if getattr(v, 'period_start_date') else None,
        Flag.func.name,
    ),
    'period_end_date': InfoTitle(
        'Период приложения - дата окончания размещения',
        lambda v: getattr(v, 'period_end_date').replace(tzinfo=None) if getattr(v, 'period_end_date') else None,
        Flag.func.name,
    ),
    'updated_at': InfoTitle(
        'Дата обновления',
        lambda v: getattr(v, 'updated_at').replace(tzinfo=None) if getattr(v, 'updated_at') else None,
        Flag.func.name,
    ),
    'signatory_one': InfoTitle('Подписант в именительном падеже', 'signatory_one', Flag.attr.name),
    'signatory_two': InfoTitle('Подписант в родительном падеже', 'signatory_two', Flag.attr.name),
    'signatory_position': InfoTitle('Должность подписанта', 'signatory_position', Flag.attr.name),
    'payment_date': InfoTitle(
        'Срок оплаты',
        lambda v: getattr(v, 'payment_date').replace(tzinfo=None) if getattr(v, 'payment_date') else None,
        Flag.func.name,
    ),
    'is_archive': InfoTitle('В архиве', lambda v: '+' if getattr(v, 'is_archive') else '-', Flag.func.name),
    'contract_id': InfoTitle('Идентификатор договора', 'contract_id', Flag.attr.name),
    'project_id': InfoTitle('Идентификатор проекта', 'project_id', Flag.attr.name),
    'creator_id': InfoTitle('Идентификатор создателя', 'creator_id', Flag.attr.name),
    'sales_manager_id': InfoTitle('Идентификатор менеджер по продажам', 'sales_manager_id', Flag.attr.name),
}


class DownloadAdvertisingSidesAndAppendicesInfo(graphene.Mutation):
    class Arguments:
        query_parameters = graphene.JSONString()
        include_fields_adv_sides = graphene.List(graphene.String)
        include_fields_appendices = graphene.List(graphene.String)

    file = graphene.String(description='Файл *.xlsx в кодировке Base64')
    ok = graphene.Boolean()

    @login_or_permissions_required(login_required=True, permissions=('api.view_advertisingside', 'api.view_appendix'))
    def mutate(root, info, **input):
        include_fields_adv_sides = input.get('include_fields_adv_sides', None)
        include_fields_appendices = input.get('include_fields_appendices', None)
        if not include_fields_adv_sides:
            adv_side_include_fields = adv_side_field_mapper.values()
        else:
            adv_side_include_fields = tuple(
                [value for key, value in adv_side_field_mapper.items() if key in include_fields_adv_sides]
            )
        if not include_fields_appendices:
            appendix_include_fields = appendices_field_mapper.values()
        else:
            appendix_include_fields = tuple(
                [value for key, value in appendices_field_mapper.items() if key in include_fields_appendices]
            )

        query_parameters = input.get('query_parameters', None)
        query_parameters = parse_parameters(query_parameters)

        construction_sides, offset, limit, count = db_query(query_parameters)

        appendices = m.Appendix.objects.filter(
            packages_reservations__package__construction_sides__in=construction_sides
        )

        constructor_sides = ConstructorXlsxQueryset(construction_sides, 'adv_sides.xlsx', adv_side_include_fields)
        constructor_appendices = ConstructorXlsxQueryset(appendices, 'appendices.xlsx', appendix_include_fields)

        constructor_sides.write()
        constructor_appendices.write()

        copy_sheet_to_workbook(constructor_sides.wb, constructor_appendices.sheet, 'Приложения')

        output = base64.b64encode(constructor_sides.get_virtual_workbook().read()).decode('ascii')
        return DownloadAdvertisingSidesAndAppendicesInfo(file=output, ok=True)
