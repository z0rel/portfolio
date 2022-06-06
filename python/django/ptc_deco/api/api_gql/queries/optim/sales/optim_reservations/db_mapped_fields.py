from collections import OrderedDict
from django.db.models import F


CONSTRUCTION_ORDER_BY = {
}


IDS_TO_UNPACK = [
    'advertising_side__side__format__model__underfamily__family__id',
    'construction__location__postcode__district__city__id',
    'construction__location__postcode__district__id',
    'construction__location__marketing_address__id',
    'construction__nonrts_owner__id',
    'package_id',
]


RELATED_ARGS_MAPPED_CONSTRUCTION_SIDE = {
    'construction__location__marketing_address__id': [['construction__location__marketing_address'], []],
    'construction__location__marketing_address__address__icontains': [
        ['construction__location__marketing_address'],
        [],
    ],
    'construction__location__legal_address__address__icontains': [['construction__location__legal_address'], []],
    'advertising_side__side__format__model__underfamily__family__id': [
        [
            'advertising_side__side__format__model__underfamily__family',
            'advertising_side__side__format__model__underfamily',
            'advertising_side__side__format__model',
            'advertising_side__side__format',
            'advertising_side__side',
            'advertising_side',
        ],
        [
            'advertising_side__side__format__model__underfamily__family__id',
        ],
    ],
    'construction__location__postcode__district__city__id': [['construction__location__postcode__district__city'], []],
    'construction__location__postcode__district__id': [['construction__location__postcode__district'], []],
    'construction__location__postcode__title__icontains': [['construction__location__postcode'], []],
    'construction__num_in_district': [['construction'], []],
    'advertising_side__code': [['advertising_side'], []],
    'advertising_side__side__code': [['advertising_side__side'], []],
    'advertising_side__side__format__code__icontains': [['advertising_side__side__format'], []],
    'advertising_side__side__format__title__icontains': [['advertising_side__side__format'], []],
    'advertising_side__side__format__title': [['advertising_side__side__format'], []],
    'advertising_side__side__title__icontains': [['advertising_side__side'], []],
    'advertising_side__side__title': [['advertising_side__side'], []],
    'advertising_side__side__size__icontains': [['advertising_side__side'], []],
    'advertising_side__side__size': [['advertising_side__side'], []],
    'construction__status_connection': [['construction'], []],
    'construction__nonrts_owner__title__icontains': [['construction__nonrts_owner'], []],
    'construction__nonrts_owner__id': [['construction__nonrts_owner'], []],
}


RELATED_ARGS_MAPPED_RESERVATION = {
    'reservation_type__title__iregex': 'reservation_type',
}


RELATED_SET_CONSTRUCTION_SIDES = {
    'advertising_side',
    'advertising_side__side',
    'advertising_side__side__format',
    'construction',
    'construction__model',
    'construction__location',
    'construction__location__marketing_address',
    'construction__location__legal_address',
    'construction__location__postcode',
    'construction__location__postcode__district',
    'construction__location__postcode__district__city',
    'construction__nonrts_owner',
    'package',
}


ONLY_FIELDS_CONSTRUCTION = [
    'id',
    'advertising_side_id',
    'advertising_side__code',
    'advertising_side__title',
    'advertising_side__side__code',
    'advertising_side__side__title',
    'advertising_side__side__size',
    'advertising_side__side__format__code',
    'advertising_side__side__format__title',
    'construction__model__title',
    'construction__status_connection',
    'construction__num_in_district',
    'construction__is_nonrts',
    'construction__nonrts_owner__title',
    'construction__location__marketing_address__address',
    'construction__location__legal_address__address',
    'construction__location__postcode__title',
    'construction__location__postcode__district__title',
    'construction__location__postcode__district__city__title',
    'package__title',
]


CONSTRUCTION_SIDE_ANNOTATION_FIELDS = OrderedDict([
    ('marketing_address', F('construction__location__marketing_address__address')),
    ('legal_address', F('construction__location__legal_address__address')),
    ('advertising_side_code', F('advertising_side__code')),
    ('advertising_side_title', F('advertising_side__title')),
    ('side_code', F('advertising_side__side__code')),
    ('side_title', F('advertising_side__side__title')),
    ('side_size', F('advertising_side__side__size')),
    ('format_code_side', F('advertising_side__side__format__code')),
    ('format_title_side', F('advertising_side__side__format__title')),
    ('format_code', F('advertising_side__side__format__code')),
    ('format_title', F('advertising_side__side__format__title')),
    ('status_connection', F('construction__status_connection')),
    ('num_in_district', F('construction__num_in_district')),
    ('is_nonrts', F('construction__is_nonrts')),
    ('nonrts_owner_title', F('construction__nonrts_owner__title')),
    ('postcode_title', F('construction__location__postcode__title')),
    ('district_title', F('construction__location__postcode__district__title')),
    ('city_title', F('construction__location__postcode__district__city__title')),
    ('package_title', F('package__title')),
])


RESERVATION_ANNOTATION_FIELDS = OrderedDict([
    ('reservation_type_title', F('reservation_type__title')),
    ('project_start_date', F('project__start_date')),
    ('project_created_at', F('project__created_at')),
    ('project_comment', F('project__comment')),
    ('project_num_in_year', F('project__num_in_year')),
    ('project_code', F('project__code')),
    ('project_title', F('project__title')),
    ('project_sales_manager_first_name', F('project__sales_manager__first_name')),
    ('project_sales_manager_last_name', F('project__sales_manager__last_name')),
    ('project_back_office_manager_first_name', F('project__back_office_manager__first_name')),
    ('project_back_office_manager_last_name', F('project__back_office_manager__last_name')),
    ('project_brand_title', F('project__brand__title'))
])


RELATED_SET_RESERVATION = {
    'reservation_type',
    'project',
    'project__sales_manager',
    'project__back_office_manager',
    'project__brand',
}


ONLY_FIELDS_RESERVATION = [
    'id',
    'date_from',
    'date_to',
    'construction_side_id',
    'reservation_type__title',
    'reservation_type_id',
    'project__title',
    'project__sales_manager__first_name',
    'project__sales_manager__last_name',
    'project__back_office_manager__first_name',
    'project__back_office_manager__last_name',
    'project__brand__title',
    'reservation_type_id',
    'project_id',
    'project__code',
    'project__sales_manager_id',
    'project__back_office_manager_id',
    'project__brand_id',
    'project__start_date',
    'project__num_in_year',
    'project__comment',
    'branding',
]
