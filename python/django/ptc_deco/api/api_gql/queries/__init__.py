import graphene
from django.contrib.postgres.aggregates import StringAgg, ArrayAgg
from django.conf import settings

from django.db.models import Prefetch, F, Q, Count, Case, When, Value

from django.contrib.auth.models import Group, Permission

from django_filters import CharFilter, NumberFilter
from django.apps import apps

from .create_node_class import cr_node_class
from ..utils.auth.decorators import login_or_permissions_required

from ....logger import models as logger_models

from ....api import models as m

# from .search_q_summary import SummaryQuery
from .optim.sales import optim_reservations, optim_projects
from .optim.sales import estimate, estimate_protobuf

from .optim.mountings import m_mobile as optim_mountings
from .optim.mountings import m_projects as mountings_projects
from .optim.mountings import m_project_card_sides as mountings_project_card
from .optim.mountings import m_project_card_companies as mountings_project_card_companies

from ..queries import search_side_size
from ..queries import search_format_titles
from ..queries import mounting_kpi
from ..queries import search_packages_info

from .search_count_of_mountings import CountOfMountings, resolve_count_of_mountings
from .optim import statistic
from graphene_django.filter import DjangoFilterConnectionField

ConstructionHistoryModel = apps.get_model(app_label='api', model_name='construction_history')
ConstructionSideHistoryModel = apps.get_model(app_label='api', model_name='construction_side_history')


def create_NodeQuery_class():
    g = cr_node_class

    src = [
        #  # logs
        [
            'search_logs',
            g(
                logger_models.ChangeLog,
                ['id'],
                login_required=True,
                permissions=('logger.view_changelog', )
              )
        ],
        # users
        ['search_user_group', g(Group, ['id'], login_required=True, permissions=('auth.view_group', ))],
        ['search_user_permission', g(Permission, ['id'], login_required=True, permissions=('auth.view_permission', ))],
        [
            'search_user',
            g(
                m.CustomUser,
                ['id', 'is_superuser'],
                [
                    'phone',
                    'email',
                    'employee_position__title',
                    'name',
                    'username',
                    'groups__name',
                    'first_name',
                    'last_name',
                ],
                login_required=True,
                permissions=('api.view_customuser', ),
            ),
        ],
        [
            'search_employee_position',
            g(
                m.EmployeePosition,
                ['id'],
                ['title'],
                login_required=True,
                permissions=('api.view_employeeposition', ),
            )
        ],
        # construction
        [
            'search_family_construction',
            g(
                m.FamilyConstruction,
                ['id'],
                ['title'],
                login_required=True,
                permissions=('api.view_familyconstruction', ),
            )
        ],
        [
            'search_under_family_construction',
            g(
                m.UnderFamilyConstruction,
                ['id', 'family__id'],
                ['title'],
                login_required=True,
                permissions=('api.view_underfamilyconstruction', ),
            )
        ],
        [
            'search_model_construction',
            g(
                m.ModelConstruction,
                ['id', 'underfamily__id'],
                ['title'],
                login_required=True,
                permissions=('api.view_modelconstruction', ),
            )
        ],
        [
            'search_format',
            g(
                m.Format,
                ['id', 'model__id', 'model__underfamily__id', 'model__underfamily__family__id'],
                [
                    'title',
                    'model__title',
                    'model__underfamily__title',
                    'model__underfamily__family__title',
                ],
                login_required=True,
                permissions=('api.view_format', ),
            ),
        ],
        [
            'search_side',
            g(
                m.Side,
                [
                    'id',
                    'format__id',
                    'format__model__id',
                    'format__model__underfamily__id',
                    'format__model__underfamily__family__id',
                ],
                [
                    'title',
                    'size',
                    'format__title',
                ],
                login_required=True,
                permissions=('api.view_side', ),
            ),
        ],
        [
            'search_advertising_side',
            g(
                m.AdvertisingSide,
                ['id', 'side__id'],
                ['title'],
                login_required=True,
                permissions=('api.view_advertisingside', ),
            )
        ],
        [
            'search_purpose_side',
            g(
                m.PurposeSide,
                ['id'],
                ['title'],
                login_required=True,
                permissions=('api.view_purposeside', ),
            )
        ],
        [
            'search_tech_problem',
            g(
                m.TechProblems,
                ['id'],
                ['title', 'comment'],
                login_required=True,
                permissions=('api.view_techproblems', ),
            )
        ],
        [
            'search_construction',
            g(
                m.Construction,
                exact_fields=[
                    'id',
                    'status_connection',
                    'active',
                    'is_nonrts',
                    'location__has_area',
                    'coordinates',
                    'crew',
                ],
                exact_icontains_fields=[
                    'location__postcode__title',
                    'location__postcode__district__title',
                    'location__postcode__district__city__title',
                    'location__postcode__district__city__country__title',
                    'location__marketing_address__address',
                    'location__legal_address__address',
                    'buh_invent_number',
                    'tech_invent_number',
                    'tech_phone_construction',
                    'model__title',
                    'model__underfamily__title',
                    'model__underfamily__family__title',
                    'obstruction__title',
                ],
                isnull_fields=['location_id'],
                exact_lt_gt_fields=['created_at', 'updated_at', 'location__area'],
                order_by=[
                    ('formats_codes_list', 'formats_codes_list'),
                    ('num_in_district', 'num_in_district'),
                    ('postcode__district__city__title', 'city'),
                    ('postcode__district__title', 'district'),
                    ('postcode__title', 'post'),
                    ('marketing_address__address', 'adress_m'),
                    ('legal_address__address', 'adress_j'),
                    ('tech_invent_number', 'inv_oto'),
                    ('buh_invent_number', 'inv_buh'),
                    ('model__title', 'model'),
                    ('model__underfamily__title', 'underfamily'),
                    ('model__underfamily__family__title', 'family'),
                    ('tech_phone_construction', 'phone'),
                    ('coordinates', 'coords'),
                    ('status_connection', 'fire'),
                    ('status_availability', 'status_availability'),
                    ('formats_list', 'format'),
                    ('formats_count', 'formats_count'),
                ],
                optim_queryset=lambda qs: (
                    qs.select_related(
                        'model',
                        'model__underfamily',
                        'model__underfamily__family',
                        'location',
                        'marketing_address',
                        'legal_address',
                        'postcode',
                        'postcode__district',
                        'postcode__district__city',
                        'location__marketing_address',
                        'location__legal_address',
                        'location__postcode',
                        'location__postcode__district',
                        'location__postcode__district__city',
                        'obstruction',
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
                ),
                extra_fields=[
                    ('formats_list', graphene.String(), lambda self, info: self.formats_list),
                    ('formats_codes_list', graphene.String(), lambda self, info: self.formats_codes_list),
                    ('formats_count', graphene.Int(), lambda self, info: self.formats_count),
                ],
                extra_filter_fields={
                    'format__title': CharFilter(max_length=255, method='filter_format_title'),
                    'filter_format_title': lambda self, queryset, name, value: (
                        queryset.filter(formats_arr__contains=[value])
                    ),
                },
                fast_search_int_fields=['row_idx'],
                fast_search_str_fields=[
                    'postcode__district__city__title',
                    'postcode__title',
                    'marketing_address__address',
                    'legal_address__address',
                    'model__title',
                    'coordinates',
                    'formats__format__title',
                    'status_connection',
                ],
                login_required=True,
                permissions=('api.view_construction', ),
            ),
        ],
        [
            'search_construction_formats',
            g(
                m.ConstructionFormats,
                exact_fields=['id'],
                exact_lt_gt_fields=['count'],
                login_required=True,
                permissions=('api.view_constructionformats', ),
            ),
        ],
        [
            'search_construction_side',
            g(
                m.ConstructionSide,
                [
                    'id',
                    'advertising_side__side__format__model__underfamily__family__id',
                    'construction_id',
                    'construction__status_connection',
                    'construction__location__postcode__district__id',
                    'construction__location__postcode__district__city__id',
                    'construction__nonrts_owner__id',
                    'advertising_side__side__format__code',
                    'advertising_side__side__code',
                    'advertising_side__code',
                    'construction__location__postcode__title',
                ],
                [
                    'construction__location__marketing_address__address',
                    'construction__location__legal_address__address',
                    'construction__location__postcode__district__title',
                    'construction__location__postcode__district__city__title',
                    'advertising_side__side__format__model__underfamily__family__title',
                    'advertising_side__side__format__model__underfamily__title',
                    'advertising_side__side__format__model__title',
                    'advertising_side__side__format__title',
                    'advertising_side__side__title',
                    'advertising_side__side__size',
                    'construction__nonrts_owner__title',
                ],
                [
                    'reservation__date_from',
                    'reservation__date_to',
                    'construction__num_in_district',
                ],
                [
                    'reservation__reservation_type__title',
                ],
                optim_queryset=lambda qs: (
                    qs.select_related(
                        'advertising_side',
                        'advertising_side__side',
                        'advertising_side__side__format',
                        'construction',
                        'construction__location',
                        'construction__location__postcode',
                        'construction__location__postcode__district',
                        'construction__location__postcode__district__city',
                    ).prefetch_related(
                        Prefetch(
                            'reservation',
                            queryset=m.Reservation.objects.select_related(
                                'project',
                                'reservation_type',
                                'project__sales_manager',
                                'project__back_office_manager',
                                'project__brand',
                            ),
                        ),
                    )
                ),
                login_required=True,
                permissions=('api.view_constructionside', ),
            ),
        ],
        [
            'search_obstruction',
            g(
                m.construction.Obstruction,
                ['id'],
                ['title'],
                login_required=True,
                permissions=('api.view_obstruction', ),
            )
        ],
        ['search_package',
            g(
                m.Package,
                ['id'],
                ['title'],
                ['year'],
                ['month'],
                login_required=True,
                permissions=('api.view_package', ),
            )
         ],
        ['search_mounting_task',
            g(
                m.MountingTask,
                ['id'],
                login_required=True,
                permissions=('api.view_mountingtask', ),
            )
         ],
        [
            'search_mounting',
            g(
                m.Mounting,
                [
                    'id',
                    'mounting_done',
                    'unmounting_done',
                ],
                [
                    'mounting_task__title',
                    'crew__name',
                    'reservation__construction_side__construction__location__legal_address__address',
                    'reservation__construction_side__construction__location__marketing_address__address',
                ],
                ['start_mounting', 'end_mounting'],
                [],
                [
                    'reservation__construction_side__construction__location__legal_address__address',
                    'reservation__construction_side__construction__location__marketing_address__address',
                ],
                login_required=True,
                permissions=('api.view_mounting', ),
            ),
        ],
        [
            'search_mounting_photo',
            g(
                m.MountingPhoto,
                ['id'],
                ['num'],
                ['date'],
                login_required=True,
                permissions=('api.view_mountingphoto', ),
            )
        ],
        [
            'search_reservation_package',
            g(
                m.ReservationPackage,
                ['id'],
                exact_lt_gt_fields=['date_from', 'date_to'],
                login_required=True,
                permissions=('api.view_reservationpackage', ),
            )
        ],
        [
            'search_contract_type',
            g(
                m.ContractType,
                ['id'],
                ['name'],
                login_required=True,
                permissions=('api.view_contracttype',),
            )
        ],
        [
            'search_contract',
            g(
                m.Contract,
                [
                    'id',
                    'return_status',
                ],
                [
                    'initiator__name',  # TODO
                    'creator__name',  # TODO
                    'partner__title',
                    'contract_type__name',
                    'code',
                    'partner__title',
                ],
                ['start', 'end', 'registration_date'],
                optim_queryset=lambda qs: (
                    qs.select_related('initiator', 'partner', 'creator', 'contract_type').annotate(
                        project_titles_list=StringAgg(
                            F('contract_appendices__project__title'),
                            delimiter=',',
                            distinct=True,
                            ordering=F('contract_appendices__project__title'),
                        )
                    )
                ),
                order_by=[  # ===========
                    ('code', 'code'),
                    ('partner__title', 'partner'),
                    ('project_titles_list', 'project'),
                    ('registration_date', 'date_start'),
                    ('end', 'date_end'),
                ],
                extra_fields=[('project_titles_list', graphene.String(), lambda self, info: self.project_titles_list)],
                fast_search_str_fields=[
                    'code',
                    'partner__title',
                    'contract_appendices__project__title',
                ],
                fast_search_date_fields=[
                    'registration_date',
                    'end'
                ],
                login_required=True,
                permissions=('api.view_contract', ),
            ),
        ],
        [
            'search_appendix',
            g(
                m.Appendix,
                [
                    'id',
                    'creator',
                    'contract_id',
                    'contract__partner_id',
                    'project__client_id',
                    'project__back_office_manager_id',
                    'sales_manager_id',
                    'project__id',
                    'project_id',
                    'created_date',
                    'return_status',
                    'project__brand_id',
                ],
                [
                    'code',
                    'project__title',
                    'project__brand__title',
                    'project__code',
                    'project__client__title',
                    'sales_manager__first_name',
                    'sales_manager__last_name',
                    'project__back_office_manager__first_name',
                    'project__back_office_manager__last_name',
                    'contract__code',
                    'contract__serial_number',
                    'contract__partner__title',
                    'project__client__title',
                ],
                ['period_start_date', 'period_end_date', 'created_date'],
                optim_queryset=lambda qs: (
                    qs.select_related('creator', 'contract', 'contract__partner', 'project', 'sales_manager')
                ),
                order_by=[  # ===========
                    ('code', 'code'),
                    ('contract__partner__title', 'partner'),
                    ('project__title', 'project'),
                    ('period_start_date', 'date_start'),
                    ('period_end_date', 'date_end'),
                    ('contract__code', 'contract__code'),
                    ('project__brand__title', 'brand'),
                    ('project__brand__working_sector__description', 'sector'),
                    ('created_date', 'create'),
                    ('creator__last_name', 'creator'),
                    ('creator__first_name', 'creator_first_name'),
                ],
                login_required=True,
                permissions=('api.view_appendix', ),
            ),
        ],
        # contragents
        [
            'search_working_sector',
            g(
                m.WorkingSector,
                ['id'],
                ['description'],
                login_required=True,
                permissions=('api.view_workingsector', ),
            )
        ],
        [
            'search_partner_type',
            g(
                m.PartnerType,
                ['id'],
                ['title'],
                login_required=True,
                permissions=('api.view_partnertype', ),
            )
        ],
        [
            'search_client_type',
            g(
                m.ClientType,
                ['id'],
                login_required=True,
                permissions=('api.view_clienttype',),
            )
        ],
        [
            'search_partner',
            g(
                m.Partner,
                ['id', 'is_nonrts_owner', 'advertisers__id'],
                [
                    'title',
                    'brands__title',
                    'advertisers__title',
                    'working_sectors__title',
                    'bin_number',
                    'partner_type__title',
                ],
                order_by=[
                    ('partner_type__title', 'type'),
                    ('title', 'partner'),
                    ('brands_list', 'brand'),
                    ('working_sectors_list', 'sector'),
                    ('client_type__title', 'client'),
                    ('bin_number', 'bin_number'),
                ],
                optim_queryset=lambda qs: (
                    qs.select_related(
                        'partner_type',
                        'client_type',
                    )
                    .prefetch_related(
                        'working_sectors',
                        'brands',
                    )
                    .annotate(
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
                ),
                extra_fields=[
                    ('brands_list', graphene.String(), lambda self, info: self.brands_list),
                    ('working_sectors_list', graphene.String(), lambda self, info: self.working_sectors_list),
                ],
                fast_search_str_fields=[
                    'projects_agencies__title',
                    'projects_agencies__code',
                    'projects_agencies__comment',
                    'title',
                    'brands__title',
                    'partner_type__title',
                    'working_sectors__title',
                    'client_type__title',
                ],
                fast_search_date_fields=[
                    'projects_agencies__created_at',
                ],
                login_required=True,
                permissions=('api.view_partner', ),
            ),
        ],
        [
            'static_additional_costs',
            g(
                m.StaticAdditionalCosts,
                ['id', 'city', 'format', 'category'],
                login_required=True,
                permissions=('api.view_staticadditionalcosts', ),
            )
        ],
        [
            'search_contact_person',
            g(
                m.ContactPerson,
                ['id'],
                login_required=True,
                permissions=('api.view_contactperson', ),
            )
        ],
        # crews
        [
            'search_crew',
            g(
                m.Crew,
                [
                    'id',
                    # 'construction__created_at',
                    # 'construction__family_construction__under_family_construction__model_construction'
                ],
                [  # Поиск по адресу
                    'city__title',
                    # 'construction__city__title',
                    # 'construction__district__title',
                    # 'construction__marketing_address',
                    # Поиск по экипажу
                    'constructions__location__postcode__district__title',
                    'constructions__location__postcode__district__city__title',
                    'constructions__location__marketing_address__address',
                    'name',
                    'phone',
                    # Поиск по конструкции
                    # 'construction__construction_side__format__title'
                ],
                optim_queryset=lambda qs: (
                    qs.prefetch_related(
                        'constructions',
                        'constructions__location__marketing_address',
                        'constructions__location__postcode__district__city',
                    ).annotate(
                        city_title=F('city__title')
                    )
                ),
                order_by=[  # ===========
                    ('id', 'key'),
                    ('num', 'num'),
                    ('name', 'name'),
                    ('phone', 'phone'),
                    ('city_title', 'city'),
                ],
                fast_search_str_fields=[
                    'city__title',
                    'constructions__formats__format__title',
                    'constructions__legal_address__address',
                ],
                fast_search_int_fields=[
                    'constructions__row_idx',
                ],
                fast_search_date_fields=[
                    'construction_sides__mountings__start_mounting',
                ],
                login_required=True,
                permissions=('api.view_crew', ),
            ),
        ],
        # geolocation
        [
            'search_loc_registration_status',
            g(
                m.RegistrationStatusLocation,
                ['id'],
                ['title', 'subcategory'],
                login_required=True,
                permissions=('api.view_registrationstatuslocation', ),
            )
        ],
        [
            'search_loc_purpose',
            g(
                m.PurposeLocation,
                ['id'],
                ['title'],
                login_required=True,
                permissions=('api.view_purposelocation', ),
            )
        ],
        [
            'search_country',
            g(
                m.Country,
                ['id'],
                ['title'],
                login_required=True,
                permissions=('api.view_country', ),
            )
        ],
        [
            'search_city',
            g(
                m.City,
                ['id', 'country__id'],
                ['title', 'country__title'],
                login_required=True,
                permissions=('api.view_city', ),
            )
        ],
        [
            'search_district',
            g(
                m.District,
                ['id', 'city__id'],
                ['title', 'city__title'],
                login_required=True,
                permissions=('api.view_district', ),
            )
        ],
        [
            'search_postcode',
            g(
                m.Postcode,
                ['id', 'district__id'],
                ['title', 'district__title', 'district__city__title'],
                [],
                login_required=True,
                permissions=('api.view_postcode', ),
            ),
        ],
        [
            'search_loc_adress',
            g(
                m.Addresses,
                ['id', 'postcode__id', 'postcode__district__id', 'postcode__district__city__id'],
                ['address', 'postcode__title', 'postcode__district__title', 'postcode__district__city__title'],
                login_required=True,
                permissions=('api.view_addresses', ),
            ),
        ],
        [
            'search_location',
            g(
                m.Location,
                exact_fields=['id', 'rent_contract_number', 'family_construction__id'],
                exact_icontains_fields=[
                    'marketing_address__address',
                    'legal_address__address',
                    'registration_status_location__title',
                    'postcode__title',
                    'postcode__district__title',
                    'postcode__district__city__title',
                    'postcode__district__city__country__title',
                    'purpose_location__title',
                    # По местоположению
                    'resolution_number',
                    'comment',
                    'cadastral_number',
                    'area_act',
                    'family_construction__title',
                ],
                exact_lt_gt_fields=[
                    'area',
                    'rent_contract_start',
                    'rent_contract_end',
                    'area_act_date',
                    'rent_contract_start',
                    'rent_contract_end'
                ],
                order_by=[
                    ('family_construction__title', 'family_construction'),  # Семейство конструкции
                    ('postcode__district__city__title', 'city'),  # Город
                    ('postcode__district__title', 'district'),  # Район
                    ('legal_address__address', 'adress_j'),  # Адрес юридический
                    ('marketing_address__address', 'marketing_address'),  # Маркетинговый адрес
                    ('cadastral_number', 'cadastral_number'),  # Кадастровый номер
                    ('area', 'area'),  # Площадь
                    # ('constructions_count', 'constructionQuantity'),  # Количество конструкций
                    ('purpose_location__title', 'purpose_location'),  # Целевое назначение
                    ('comment', 'comment'),  # Коментарий
                    ('has_area', 'has_area'),  # Наличие земли
                    # TODO: M2M связь с координатами, дорабатывать загрузчик конструкций
                    # ('coordinate', 'coordinates'),  # Координаты
                    ('resolution_number', 'resolution_number'),  # Номер постановления от Акимата
                    ('resolution_number_date', 'resolution_number_date'),  # Дата постановления от Акимата
                    ('area_act', 'areaAct'),  # Номер гос акта на землю
                    ('area_act_date', 'area_act_date'),  # Дата гос акта на землю
                    ('rent_contract_number', 'rent_contract_number'),  # Номер договора
                    ('rent_contract_start', 'rent_contract_start'),  # Дата начала договора
                    ('rent_contract_end', 'rent_contract_end'),  # Дата окончания договора
                    ('rent_contract_created_at', 'rent_contract_createdAt'),  # Регистрация договора
                    ('created_at', 'created_at'),  # Дата создания
                    ('updated_at', 'updated_at'),  # Дата обновления
                    (
                        'registration_status_location__title',
                        'registration_status_location',
                    ),  # Статус оформления земельного участка
                    ('constructions_count', 'constructions_count'),
                ],
                optim_queryset=lambda qs: (
                    qs.select_related(
                        'family_construction',
                        'registration_status_location',
                        'marketing_address',
                        'legal_address',
                        'purpose_location',
                        'postcode__district',
                        'postcode__district__city',
                        'postcode__district__city__country',
                    )
                    .prefetch_related('constructions')
                    .annotate(constructions_count=Count(F('constructions')))
                ),
                extra_fields=[('constructions_count', graphene.Int(), lambda self, info: self.constructions_count)],
                extra_filter_fields={
                    'constructions_count__gt': NumberFilter(method='filter_constructions_count__gt'),
                    'filter_constructions_count__gt': lambda self, queryset, name, value: (
                        queryset.filter(constructions_count__gt=value)
                    ),
                    'constructions_count__lt': NumberFilter(method='filter_constructions_count__lt'),
                    'filter_constructions_count__lt': lambda self, queryset, name, value: (
                        queryset.filter(constructions_count__lt=value)
                    ),
                    'constructions_count': NumberFilter(method='filter_constructions_count'),
                    'filter_constructions_count': lambda self, queryset, name, value: (
                        queryset.filter(constructions_count=value)
                    ),
                },
                fast_search_str_fields=[
                    'family_construction__title',
                    'cadastral_number',
                    'postcode__district__title',
                    'postcode__district__city__title',
                    'legal_address__address',
                    'area',
                    # если фильтровать area по string, то можно будет искать подобное (3.1277), но при этом
                    # если в бд поле area содержит 3.1277 и в fastSearch вводится 122 то в результате вернет
                    # запись, которая содержит 122, но при этом на самом деле area может быть и 3.122 и 1512.122
                    # при необходимости - перенести в fast_search_int_fields и фильтрация будет работать по целой части
                ],
                fast_search_count_fields=[
                    'constructions__location',
                ],
                login_required=True,
                permissions=('api.view_location', ),
            ),
        ],
        # project
        [
            'search_agency_commission',
            g(
                m.AgencyCommission,
                ['id'],
                login_required=True,
                permissions=('api.view_agencycommission', ),
            )
        ],
        [
            'search_project_cities',
            g(
                m.ProjectCities,
                ['id'],
                login_required=True,
                permissions=('api.view_projectcities', ),
            )
        ],
        [
            'search_project',
            g(
                m.Project,
                [
                    'id',
                    'brand__id',
                    'client__id',
                    'client__partner_type__id',
                    'client__working_sectors__id',
                    'back_office_manager__id',
                    'sales_manager__id',
                    'num_in_year',
                ],
                [
                    'code',
                    'title',
                    'brand__title',
                    'client__title',
                    'client__partner_type__title',
                    'client__working_sectors__title',
                    'client__working_sectors__description',
                    'sales_manager__name',
                    'sales_manager__first_name',
                    'sales_manager__last_name',
                    'sales_manager__email',
                    'sales_manager__phone',
                    'back_office_manager__name',
                    'back_office_manager__email',
                    'back_office_manager__phone',
                    'back_office_manager__first_name',
                    'back_office_manager__last_name',
                ],
                [
                    'start_date',
                ],
                fast_search_str_fields=[
                    'project_appendices__contract__code',
                    'project_appendices__code',
                    'brand__working_sector__title',
                    'brand__title',
                    'creator__name',
                ],
                fast_search_date_fields=[
                    'created_at',
                ],
                login_required=True,
                permissions=('api.view_project', ),
            ),
        ],
        [
            'search_advert_promo_companies',
            g(
                m.AdvertPromoCompany,
                [
                    'id',
                    'project_id',
                    'project__project_cities__city_id',
                    'project__client_id',
                    'project__brand_id',
                    'designs__is_current',
                    'designs__archived',
                ],
                [
                    'project__client__title',
                    'project__brand__title',
                    'project__project_cities__city__title',
                ],
                exact_lt_gt_fields=[
                    'project__project_cities__count',
                    'project__project_cities__saled_count',
                    'project__project_cities__distributed_count',
                    'designs__started_at',
                ],
                optim_queryset=lambda qs: (
                    qs.prefetch_related('designs', 'project__project_cities')
                    .select_related('project', 'project', 'project__client', 'project__brand')
                    .annotate(
                        count_of_all_designs=Count(F('designs')),
                        count_of_archived_designs=Count(F('designs'), filter=Q(designs__archived=True)),
                        city_title=F('city__title'),
                    )
                ),
                extra_fields=[
                    ('count_of_all_designs', graphene.Int(), lambda self, info: self.count_of_all_designs),
                    ('count_of_archived_designs', graphene.Int(), lambda self, info: self.count_of_archived_designs),
                ],
                order_by=[  # ===========
                    ('id', 'code'),
                    ('start', 'start'),
                    ('title', 'title'),
                    ('city_title', 'city_title'),
                    ('count_of_all_designs', 'count'),
                    ('count_of_archived_designs', 'count_arhive'),
                ],
                login_required=True,
                permissions=('api.view_advertpromocompany', ),
            ),
        ],
        [
            'search_reservation_type',
            g(
                m.ReservationType,
                ['id'],
                ['title'],
                login_required=True,
                permissions=('api.view_reservationtype', ),
            )
        ],
        [
            'search_reservation',
            g(
                m.Reservation,
                [
                    'id',
                    'project_id',
                    'appendix__id',
                    'construction_side__advertising_side__side__format__model__underfamily__family__id',
                    'construction_side__construction__status_connection',
                    'construction_side__construction__location__postcode__district__id',
                    'construction_side__construction__location__postcode__district__city__id',
                ],
                [
                    'construction_side__construction__location__postcode__district__title',
                    'construction_side__construction__location__postcode__district__city__title',
                    'construction_side__advertising_side__side__format__model__underfamily__family__title',
                    'construction_side__advertising_side__side__format__model__underfamily__title',
                    'construction_side__advertising_side__side__format__model__title',
                    'construction_side__advertising_side__side__format__title',
                    'construction_side__advertising_side__side__title',
                    'construction_side__advertising_side__side__size',
                ],
                [
                    'date_from',
                    'date_to',
                ],
                [
                    'reservation_type__title',
                ],
                optim_queryset=lambda qs: (
                    qs.select_related(
                        'construction_side__advertising_side__side__format__model__underfamily__family',
                        'construction_side__construction__location__postcode__district__city',
                        'reservation_type',
                        'project__sales_manager',
                        'project__back_office_manager',
                        'project__brand',
                    )
                ),
                order_by=[
                    ('construction_side__construction__location__postcode__title', 'postcode_title'),  # Код стороны
                    ('construction_side__construction__num_in_district', 'num_in_district'),
                    ('construction_side__advertising_side__side__format__code', 'format_code'),
                    ('construction_side__advertising_side__side__code', 'side_code'),
                    ('construction_side__advertising_side__code', 'adv_side_code'),
                    (
                        'construction_side__construction__location__postcode__district__city__title',
                        'reservation_city',
                    ),  # Город
                    (
                        'construction_side__construction__location__marketing_address__address',
                        'reservation_address',
                    ),  # Адрес
                    ('construction_side__advertising_side__side__format__title', 'reservation_format'),  # Формат
                    ('construction_side__advertising_side__side__title', 'reservation_side'),  # Сторона
                    ('creation_date', 'reservation_creation_date'),  # Дата создания
                    ('date_from', 'reservation_start_date'),  # Дата начала
                    ('date_to', 'reservation_expiration_date'),  # Дата окончания
                    ('reservation_type__title', 'reservation_status'),  # Статус
                    # ('reservation_renewalOfReservation'),  # Продление брони
                    ('branding', 'reservation_branding'),  # Брендирование
                    ('construction_side__construction__status_connection', 'reservation_lighting'),  # Освещение
                    ('construction_side__package__title', 'reservation_package'),  # Пакет
                    # ('reservation_design'),                # Дизайн
                ],
                fast_search_str_fields=[
                    'branding',
                    'construction_side__construction__location__postcode__district__city__title',
                    'construction_side__construction__location__marketing_address__address',
                    'construction_side__construction__location__postcode__title',
                    'construction_side__advertising_side__side__format__title',
                    'construction_side__advertising_side__side__format__code',
                    'construction_side__advertising_side__side__code',
                    'construction_side__advertising_side__code',
                ],
                fast_search_int_fields=[
                    'construction_side__construction__num_in_district',

                ],
                fast_search_date_fields=[
                    'date_from',
                    'date_to',
                    'creation_date',
                ],
                login_required=True,
                permissions=('api.view_reservation', ),
            ),
        ],
        [
            'search_design',
            g(
                m.Design,
                ['id', 'advert_promo_company_id', 'advert_promo_company_id', 'advert_promo_company__project_id'],
                ['title'],
                fast_search_str_fields=[
                    'title',
                ],
                fast_search_date_fields=[
                    'started_at',
                ],
                login_required=True,
                permissions=('api.view_design', ),
            ),
        ],
        [
            'search_brand',
            g(
                m.Brand,
                ['id', 'partners__id'],
                [
                    'title',
                    'partners__title',
                    'working_sector__description',
                ],
                order_by=[
                    ('title', 'brand'),
                    ('partners_list', 'partner'),
                    ('working_sector__description', 'working_sector'),
                ],
                optim_queryset=lambda qs: (
                    qs.prefetch_related(
                        'partners',
                        'partners__working_sectors',
                    )
                    .select_related('working_sector')
                    .annotate(
                        partners_list=StringAgg(
                            F('partners__title'),
                            delimiter=',',
                            distinct=True,
                            ordering=F('partners__title'),
                        )
                    )
                ),
                extra_fields=[('partners_list', graphene.String(), lambda self, info: self.partners_list)],
                fast_search_str_fields=['title', 'working_sector__title', 'partners__title'],
                # extra_filter_fields={
                #     'partners__icontains': CharFilter(max_length=255, method='filter_partners_icontains'),
                #     'filter_partners_icontains': lambda self, queryset, name, value: (
                #         queryset.filter(partners_list__icontains=value)
                #     )
                # }
                login_required=True,
                permissions=('api.view_brand', ),
            ),
        ],
        ['search_brand_image', g(m.BrandImage, ['id'], login_required=True, permissions=('api.view_brandimage', ))],
        # sales
        [
            'search_sales_additional_cost',
            g(
                m.AdditionalCosts,
                ['id', 'project_id', 'appendix'],
                [
                    'city__title',
                    'project__code',
                    'appendix__code',
                ],
                login_required=True,
                permissions=('api.view_additionalcosts', ),
            ),
        ],
        ['search_sales_nonrts', g(m.EstimateNonRts, ['id'])],
        [
            'search_sales_invoice',
            g(
                m.Invoice, ['id'],
                fast_search_str_fields=[
                    'project__code',
                    'project__title',
                    'appendix__code',
                    'contract__partner__brands__title',
                    'contract__partner__partner_type__title',
                    'project__agency__partner_type__title',
                    'customer_payment_method',
                    'partner__district__city__title',
                    'avr',
                ],
                fast_search_date_fields=[
                    'project.start_date',
                    'payment_last_date',
                ],
                fast_search_int_fields=[
                    'sum_without_nds',
                    'whole_sum',
                ],
                login_required=True,
                permissions=('api.view_invoice', ),
            )
        ],
        [
            'search_placement_price',
            g(
                m.PlacementPrice,
                ['id', 'city', 'format', 'period'],
                login_required=True,
                permissions=('api.view_placementprice', ),
            )
        ],
        [
            'search_user_notifications',
            g(
                m.Notification,
                exact_fields=[
                    'user__id'
                ],
                order_by=[
                    ('topic', 'topic'),
                    ('read', 'read'),
                    ('created_at', 'created_at'),
                    ('updated_at', 'updated_at'),
                ],
                optim_queryset=lambda qs: qs,
                login_required=True,
                permissions=('api.view_notification', ),
            )
        ],
        [
            'search_user_construction_notifications',
            g(
                m.ConstructionNotification,
                exact_fields=[
                    'user__id'
                ],
                order_by=[
                    ('id', 'id'),
                    ('construction_id', 'construction_id'),
                    ('topic', 'topic'),
                    ('read', 'read'),
                    ('created_at', 'created_at'),
                    ('updated_at', 'updated_at'),
                ],
                optim_queryset=lambda qs: (
                    qs.prefetch_related('construction')
                ),
                login_required=True,
                permissions=('api.view_constructionnotification', ),
            )
        ],
        [
            'list_construction_changes_history',
            g(
                ConstructionHistoryModel,
                ['target_id'],
                optim_queryset=lambda qs: (
                    qs.annotate(
                        manipulation_type_annotated=Case(
                            When(manipulation_type="num_in_district",
                                 then=Value('Порядковый номер конструкции в районе')),
                            When(manipulation_type="coordinates", then=Value('Координаты')),
                            When(manipulation_type="back_comment", then=Value('Комментарий')),
                            When(manipulation_type="tech_invent_number", then=Value('Инвентарный номер - Техотдел')),
                            When(manipulation_type="buh_invent_number", then=Value('Инвентарный номер - 1C')),
                            When(manipulation_type="tech_phone_construction", then=Value('Номер телефона конструкции')),
                            When(manipulation_type="status_connection", then=Value('Статус по подключению')),
                            When(manipulation_type="status_availability", then=Value('Статус доступности конструкции')),
                            When(manipulation_type="obstruction", then=Value('Конструкции -> Помеха')),
                            When(manipulation_type="tech_problem", then=Value('Конструкция -> Технические проблемы')),
                            When(manipulation_type="photo", then=Value('Изображение')),
                            When(manipulation_type="active", then=Value('Активная или демонтированная')),
                            When(manipulation_type="created_at", then=Value('Дата создания')),
                            When(manipulation_type="updated_at", then=Value('Дата обновления')),
                            When(manipulation_type="presentation_url",
                                 then=Value('Ссылка на сайт с презентацией конструкции')),
                            When(manipulation_type="is_archive", then=Value('В архиве')),
                            When(manipulation_type="model", then=Value('Конструкции -> Модель конструкции')),
                            When(manipulation_type="crew", then=Value('Конструкции -> Экипаж')),
                            When(manipulation_type="crews_has_special_for_sides",
                                 then=Value("Сторонам конструкции присвоены специализированные экипажи")),
                            When(manipulation_type="location", then=Value('Конструкции -> Местоположение')),
                            When(manipulation_type="is_nonrts", then=Value('Конструкция НОН РТС')),
                            When(manipulation_type="nonrts_owner", then=Value('Конструкции -> Владелец НОН РТС')),
                            When(manipulation_type="nonrts_owner_comment",
                                 then=Value('Коментарий о владельце НОН РТС')),
                            When(manipulation_type="marketing_address",
                                 then=Value('Местоположения -> Маркетинговый адрес')),
                            When(manipulation_type="legal_address", then=Value('Местоположения -> Юридический адрес')),
                            When(manipulation_type="postcode", then=Value('Местоположения -> Почтовый индекс')),
                        )
                    )
                ),
                extra_fields=[
                    ('manipulation_type_annotated', graphene.String(), lambda self, info: self.manipulation_type_annotated)
                ],
                login_required=True,
                permissions=('api.view_construction_history', ),
            )
        ],
        [
            'list_construction_side_changes_history',
            g(
                ConstructionSideHistoryModel,
                ['target_id'],
                optim_queryset=lambda qs: (
                    qs.annotate(
                        manipulation_type_annotated=Case(
                            When(manipulation_type="created_at", then=Value('Дата создания')),
                            When(manipulation_type="updated_at", then=Value('Дата обновления')),
                            When(manipulation_type="availability_side", then=Value('Статус доступности стороны')),
                            When(manipulation_type="is_archive", then=Value('В архиве')),
                            When(manipulation_type="sale_constraint", then=Value('Ограничение стороны конструкции по продажам')),
                            When(manipulation_type="construction", then=Value('Стророны конструкции -> Конструкция')),
                            When(manipulation_type="advertising_side", then=Value('Стророны конструкции -> Рекламная сторона')),
                            When(manipulation_type="purpose_side", then=Value('Стророны конструкции -> Назначение стороны')),
                            When(manipulation_type="package", then=Value('Стророны конструкции -> Пакет')),
                            When(manipulation_type="crew", then=Value('Конструкции -> Экипаж')),
                        )
                    )
                ),
                extra_fields=[
                    ('manipulation_type_annotated', graphene.String(),
                     lambda self, info: self.manipulation_type_annotated)
                ],
                login_required=True,
                permissions=('api.view_construction_side_history', ),
            )
        ],
        # ], skip_filterset_assertion=True)]
    ]

    attrs = {}
    for [k, (fast_search, default_query, unoptimized_query, model_class)] in src:
        descr = model_class._meta.verbose_name_plural
        if fast_search:
            attrs[k] = DjangoFilterConnectionField(default_query, description=descr, fast_search=graphene.String())
        else:
            attrs[k] = DjangoFilterConnectionField(default_query, description=descr)
        if unoptimized_query and fast_search:
            attrs[k + '_unoptimized'] = DjangoFilterConnectionField(
                unoptimized_query, description=descr + ' (неоптимизированный запрос)', fast_search=graphene.String()
            )
        elif unoptimized_query:
            attrs[k + '_unoptimized'] = DjangoFilterConnectionField(
                unoptimized_query, description=descr + ' (неоптимизированный запрос)'
            )

    # attrs['search_sales_estimate_itogs'] = estimate.field_django_filter_connection
    login_required = not settings.DISABLE_GQL_AUTH_CONTROL

    optim_fields = [
        (
            'search_sales_estimate_proto',
            estimate_protobuf.resolve_estimate,
            estimate_protobuf.field_estimate,
            login_required,
            ('api.view_project', 'api.view_appendix', ),  # возможно следует заменить
        ),
        (
            'search_sales_address_program_proto',
            estimate_protobuf.resolve_address_programm,
            estimate_protobuf.field_address_programm,
            login_required,
            ('api.view_project', 'api.view_appendix', ),  # возможно следует заменить
        ),
        (
            'search_advertising_sides_optim',
            optim_reservations.resolve_advertising_sides,
            optim_reservations.advertising_sides_field,
            login_required,
            ('api.view_advertisingside', ),
        ),
        (
            'search_projects_optim',
            optim_projects.resolve_projects,
            optim_projects.projects_field,
            login_required,
            ('api.view_project', ),
        ),
        (
            'search_app_mounting_json',
            optim_mountings.resolve_mountings,
            optim_mountings.mountings_field,
            login_required,
            ('api.view_mounting', ),
        ),
        (
            'search_app_mounting_gql',
            optim_mountings.resolve_debug_mountings,
            optim_mountings.mountings_debug_field,
            login_required,
            ('api.view_mounting', ),
        ),
        (
            'search_count_of_mountings',
            resolve_count_of_mountings,
            graphene.Field(CountOfMountings, description='Количество незавершенных монтажей'),
            login_required,
            ('api.view_mounting', ),
        ),
        (
            'search_mountings_projects',
            mountings_projects.resolve_mountings_projects,
            mountings_projects.field,
            login_required,
            ('api.view_mounting', 'api.view_project', 'api.view_city', 'api.view_side'),
        ),
        (
            'search_mountings_project_card',
            mountings_project_card.resolve_mountings_project_card,
            mountings_project_card.field,
            login_required,
            ('api.view_mounting', 'api.view_project', ),
        ),
        (
            'search_mountings_project_card_companies',
            mountings_project_card_companies.resolve_project_card_companies,
            mountings_project_card_companies.field,
            login_required,
            ('api.view_mounting', 'api.view_city', 'api.view_design', 'api.view_advertpromocompany', ),
        ),
        # (
        #     'search_mountings_project_card_proto',
        #     mountings_project_card.resolve_mounting_project_card_protobuf,
        #     mountings_project_card.field_proto,
        # ),
        (
            'search_side_size',
            search_side_size.resolve_search_side_size,
            search_side_size.search_side_size_field,
            login_required,
            ('api.view_side', 'api.view_format', ),
        ),
        (
            'search_side_titles',
            search_side_size.resolve_search_sides,
            search_side_size.search_side_size_field,
            login_required,
            ('api.view_side', 'api.view_format', ),
        ),
        (
            'search_format_titles',
            search_format_titles.resolve_search_formats,
            search_format_titles.search_format_titles_field,
            login_required,
            ('api.view_side', 'api.view_format', 'api.view_familyconstruction', ),
        ),
        (
            'kpi_by_crew_id',
            mounting_kpi.resolve_count_of_mountings_by_crew_id,
            mounting_kpi.MountingKPI.count_of_mountings_by_crew_id,
            login_required,
            ('api.view_mounting', ),
        ),
        (
            'search_user_statistic',
            statistic.resolve_statistic,
            statistic.CustomUserStatisticQuery.statistic,
            login_required,
            ('api.view_project', 'api.view_partner', 'api.view_user', ),
        ),
        (
            'search_packages_info',
            search_packages_info.resolve_packages_info,
            search_packages_info.packages_info_field,
            login_required,
            ('api.view_package', 'api.view_constructionside', ),
        ),
    ]

    for fieldname, resolver, field, login_required, permissions in optim_fields:
        attrs[fieldname] = field
        attrs['resolve_' + fieldname] = login_or_permissions_required(
            login_required=login_required,
            permissions=permissions
        )(resolver)

    return type('NodeQuery', (graphene.ObjectType,), attrs)


NodeQuery = create_NodeQuery_class()
