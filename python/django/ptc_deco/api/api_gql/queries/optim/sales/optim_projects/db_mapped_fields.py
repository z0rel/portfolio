from collections import OrderedDict

from django.db.models import F

MAPPED_KWARGS_WORKING_SECTORS = {
    'client__working_sectors__description__icontains': ['description__icontains', False],
    'client__working_sector_id': ['id', True],
}


MAPPED_PROJECTS_KWARGS = {
    'project_id': ['id', True],
    'brand_id': ['brand_id', True],
    'client_id': ['client_id', True],
    'agency_id': ['agency_id', True],
    'back_office_manager_id': ['back_office_manager_id', True],
    'sales_manager_id': ['sales_manager_id', True],
    'working_sector_id': ['brand__working_sector_id', True]
}


RELATED_TABLES = [
    'brand',
    'client',
    'back_office_manager',
    'sales_manager',
    'agency',
    'client__actual_address_postcode__district__city',
    'agency__actual_address_postcode__district__city',
    'brand__working_sector'
]


ONLY_FIELDS = [
    'id',
    'code',
    'num_in_year',
    'title',
    'start_date',
    'created_at',
    'brand__title',
    'client__title',
    'client__actual_address_postcode__district__city__title',
    'agency__title',
    'agency__actual_address_postcode__district__city__title',
    'back_office_manager__first_name',
    'back_office_manager__last_name',
    'sales_manager__first_name',
    'sales_manager__last_name',
    'client__working_sectors__description',
    'brand__working_sector__description'
]


ANNOTATION_FIELDS = OrderedDict([
    ('client_city_title', F('client__actual_address_postcode__district__city__title')),
    ('agency_city_title', F('agency__actual_address_postcode__district__city__title')),
    ('client_title', F('client__title')),
    ('agency_title', F('agency__title')),
    ('back_office_manager_first_name', F('back_office_manager__first_name')),
    ('back_office_manager_last_name', F('back_office_manager__last_name')),
    ('sales_manager_first_name', F('sales_manager__first_name')),
    ('sales_manager_last_name', F('sales_manager__last_name')),
    ('client_title', F('client__title')),
    ('brand_title', F('brand__title')),
    ('working_sector_title', F('brand__working_sector__description')),
    ('working_sector_id', F('brand__working_sector_id')),
    ('agency_title', F('agency__title')),
])