from collections import OrderedDict
from itertools import chain
from typing import List, Tuple, Any, Set, Optional

from django.contrib.postgres.aggregates import StringAgg
from django.db.models import Q, F, Case, When, Prefetch, QuerySet

from .......api import models as m
from ....utils import convert_ID_to_id, get_offset_limit, construction_side_code_to_filterspec
from ...generate_order_by_class import get_order_by_list, get_fast_search_param
from .order_by import ORDERING_FIELDS


LIMIT = 500


def get_annotated_format_param(param):
    return Case(
        When(reservation__isnull=False, then=F('reservation__construction_side__advertising_side__side__' + param)),
        default=F('construction_side__advertising_side__side__' + param),
    )


def get_annotated_side_param(param):
    return Case(
        When(reservation__isnull=False, then=F('reservation__construction_side__' + param)),
        default=F('construction_side__' + param),
    )


def get_annotated_construction_param(param):
    return Case(
        When(reservation__isnull=False, then=F('reservation__construction_side__construction__' + param)),
        When(construction_side__isnull=False, then=F('construction_side__construction__' + param)),
        default=F('construction__' + param),
    )


def get_annotated_construction_param_q(param, arg):
    return Case(
        When(reservation__isnull=False, then=Q(**{'reservation__construction_side__construction__' + param: arg})),
        When(construction_side__isnull=False, then=Q(**{'construction_side__construction__' + param: arg})),
        default=Q(**{'construction__' + param: arg}),
    )


def mounting_construction_related(prefix):
    return [
        prefix + 'construction__nonrts_owner',
        prefix + 'construction__location__marketing_address',
        prefix + 'construction__location__legal_address',
        prefix + 'construction__location__postcode__district__city',
    ] + (
        [prefix + 'construction_side__advertising_side__side__format__model__underfamily__family'] if not prefix else []
    )


def format_related(prefix):
    return [
        prefix + 'construction_side__advertising_side__side__format__model__underfamily__family',
        *mounting_construction_related(prefix + 'construction_side__'),
    ]


MOUNTING_SELECT_RELATED = [
    'mounting_task',
    'reservation',
    'reservation__project',
    'construction_side',
    'construction_side__construction',
    'construction',
    'design',
    'crew',
    'crew__city',
    *format_related(''),
    *format_related('reservation__'),
    *mounting_construction_related(''),
]


def get_mounting_construction_fields(prefix):
    return [
        prefix + 'construction__nonrts_owner_id',
        prefix + 'construction__nonrts_owner__title',
        prefix + 'construction__status_connection',
        prefix + 'construction__num_in_district',
        prefix + 'construction__location__postcode__title',
        prefix + 'construction__location__postcode__district__title',
        prefix + 'construction__location__postcode__district__city__title',
        prefix + 'construction__location__marketing_address__address',
        prefix + 'construction__location__legal_address__address',
        # prefix + 'construction__format__title',
        # prefix + 'construction__format__code',
        prefix + 'construction__model__title',
        prefix + 'construction__model__underfamily__title',
        prefix + 'construction__model__underfamily__title',
        prefix + 'construction__model__underfamily__family__title',
    ]


def get_mounting_only_fields(prefix):
    return [
        *get_mounting_construction_fields(prefix + 'construction_side__'),
        prefix + 'construction_side__advertising_side__title',
        prefix + 'construction_side__advertising_side__code',
        prefix + 'construction_side__advertising_side__side__title',
        prefix + 'construction_side__advertising_side__side__size',
        prefix + 'construction_side__advertising_side__side__code',
        prefix + 'construction_side__advertising_side__side__format__title',
        prefix + 'construction_side__advertising_side__side__format__code',
        prefix + 'construction_side__advertising_side__side__format__model__title',
        prefix + 'construction_side__advertising_side__side__format__model__underfamily__title',
        prefix + 'construction_side__advertising_side__side__format__model__underfamily__family__title',
    ]


def get_construction_q(field, value):
    return (
        Q(**{'construction__' + field: value})
        | Q(**{'construction_side__construction__' + field: value})
        | Q(**{'reservation__construction_side__construction__' + field: value})
    )


def get_construction_side_q(field, value):
    return Q(**{'construction_side__' + field: value}) | Q(**{'reservation__construction_side__' + field: value})


def get_construction_side_q_related(field):
    return ['construction_side__' + field, 'reservation__construction_side__' + field]


def get_construction_q_related(field):
    return [
        'construction__' + field,
        'construction_side__construction__' + field,
        'reservation__construction_side__construction__' + field,
    ]


def transform_construction_filter_value(filters, related, only, table, field, op, value):
    if value:
        filters.append(get_construction_q(table + '__' + field + op, value))
        related.extend(get_construction_q_related(table))
        only.extend(get_construction_q_related(table + '__' + field))


def transform_construction_side_filter_value(filters, related, only, table, field, op, value):
    if value:
        filters.append(get_construction_side_q(table + '__' + field + op, value))
        related.extend(get_construction_side_q_related(table))
        only.extend(get_construction_side_q_related(table + '__' + field))


FILTER_FIELDS_OF_CONSTURCTION = [
    # is_id, query field name, field_name, table, op
    [True, 'city_id', 'city_id', 'location__postcode__district', ''],
    [False, 'city_title', 'title', 'location__postcode__district__city', ''],
    [False, 'city_title__icontains', 'title', 'location__postcode__district__city', '__icontains'],
    [True, 'district_id', 'district_id', 'location__postcode', ''],
    [True, 'postcode_id', 'postcode_id', 'location', ''],
]

FILTER_FIELDS_OF_CONSTURCTION_SIDE = [
    [False, 'format_title', 'title', 'advertising_side__side__format', ''],
    [True, 'format_id', 'format_id', 'advertising_side__side', ''],
]


def get_query_construction_annotated_fields(
    get_annotated_format_param, get_annotated_side_param, get_annotated_construction_param, prefix
) -> List[Tuple[str, Any]]:
    return [
        ('format_code', get_annotated_format_param('format__code')),
        ('format_title', get_annotated_format_param('format__title')),
        ('model_title', get_annotated_construction_param('model__title')),
        ('underfamily_title', get_annotated_construction_param('model__underfamily__title')),
        ('family_title', get_annotated_construction_param('model__underfamily__family__title')),
        ('advertising_side_title', get_annotated_side_param('advertising_side__title')),
        ('advertising_side_code', get_annotated_side_param('advertising_side__code')),
        ('side_title', get_annotated_side_param('advertising_side__side__title')),
        ('side_size', get_annotated_side_param('advertising_side__side__size')),
        ('side_code', get_annotated_side_param('advertising_side__side__code')),
        ('status_connection', get_annotated_construction_param('status_connection')),
        ('num_in_district', get_annotated_construction_param('num_in_district')),
        ('postcode_title', get_annotated_construction_param('location__postcode__title')),
        ('district_title', get_annotated_construction_param('location__postcode__district__title')),
        ('city_title', get_annotated_construction_param('location__postcode__district__city__title')),
        ('address_marketing', get_annotated_construction_param('location__marketing_address__address')),
        ('address_legal', get_annotated_construction_param('location__legal_address__address')),
        ('nonrts_owner', get_annotated_construction_param('nonrts_owner__title')),
        ('reservation_construction_side_id', F(prefix + 'construction_side_id')),
        ('project_title', F(prefix + 'project__title')),
        ('project_code', F(prefix + 'project__code')),
        ('appendix_code', F(prefix + 'appendix__code')),
    ]


class AnnotationOfQueryConstructionFields:
    format_code: type(m.Format.code)
    format_title: type(m.Format.title)
    model_title: type(m.ModelConstruction.title)
    underfamily_title: type(m.UnderFamilyConstruction.title)
    family_title: type(m.FamilyConstruction.title)
    advertising_side_title: type(m.AdvertisingSide.title)
    advertising_side_code: type(m.AdvertisingSide.code)
    side_title: type(m.Side.title)
    side_size: type(m.Side.size)
    side_code: type(m.Side.code)
    status_connection: type(m.Construction.status_connection)
    num_in_district: type(m.Construction.num_in_district)
    postcode_title: type(m.Postcode.title)
    district_title: type(m.District.title)
    city_title: type(m.City.title)
    address_marketing: type(m.Addresses.address)
    address_legal: type(m.Addresses.address)
    nonrts_owner: type(m.Partner.title)
    reservation_construction_side_id: int
    project_title: type(m.Project.title)
    project_code: type(m.Project.code)


DB_QUERY_COMMON_ANNOTATED_FIELDS = OrderedDict(
    [
        *get_query_construction_annotated_fields(
            get_annotated_format_param, get_annotated_side_param, get_annotated_construction_param, 'reservation__'
        ),
        (
            'selected_construction_id',
            Case(
                When(
                    Q(reservation__isnull=False, reservation__construction_side__isnull=False),
                    then=F('reservation__construction_side__construction_id'),
                ),
                When(construction_side__isnull=False, then=F('construction_side__construction_id')),
                default=F('construction_id'),
            ),
        ),
    ]
)


MOBILE_QUERY_FIELDS = OrderedDict(
    [
        ('mounting_task_title', F('mounting_task__title')),
        ('design_img', F('design__img')),
        ('design_started_at', F('design__started_at')),
        ('design_title', F('design__title')),
        ('unmounting_design_img', F('unmounting_design__img')),
        ('unmounting_design_started_at', F('unmounting_design__started_at')),
        ('unmounting_design_title', F('unmounting_design__title')),
        ('crew_num', F('crew__num')),
        ('crew_name', F('crew__name')),
        ('crew_phone', F('crew__phone')),
        ('crew_city', F('crew__city__title')),
    ]
)


class AnnotationMobileQueryFields:
    mounting_task_title: type(m.MountingTask.title)
    design_img: type(m.Design.img)
    design_started_at: type(m.Design.started_at)
    design_title: type(m.Design.title)
    unmounting_design_img: type(m.Design.img)
    unmounting_design_started_at: type(m.Design.started_at)
    unmounting_design_title: type(m.Design.title)
    crew_num: type(m.Crew.num)
    crew_name: type(m.Crew.name)
    crew_phone: type(m.Crew.phone)
    crew_city: type(m.City.title)


MOBILE_QUERY_COMMON_ANNOTATION_FIELDS = OrderedDict(
    [
        *DB_QUERY_COMMON_ANNOTATED_FIELDS.items(),
        *MOBILE_QUERY_FIELDS.items()
        # TODO: бронирование может быть добавлено в несколько приложений, связь M2M.
        #  Простая аннотация поломает модель выдачи
        # ('appendix_code', F('reservation__appendix__code')),
    ]
)

MOBILE_QUERY_ORDERING_ANNOTATION_FIELDS = OrderedDict(
    [
        *MOBILE_QUERY_COMMON_ANNOTATION_FIELDS.items(),
        ('photo_names', StringAgg(F('photos__num'), ',', distinct=True, ordering='photos__num')),
    ]
)


ONLY_MOUNTING_LIST = [
    'id',
    'archived',
    'start_mounting',
    'end_mounting',
    'mounting_range',
    'mounting_done',
    'unmounting_done',
    'downloaded_early',
    'comment',
    'reservation_id',
    'construction_side_id',
    'construction_id',
    'design_id',
    'crew_id',
]


class DbQueryCommon:
    fast_filter: List[Q]  # фильтр запроса быстрого поиска
    related_set: Set[str]
    additional_filters: List[Q]
    additional_only_fields: List[str]
    order_by_list: List[str]
    offset: int
    limit: int
    query: QuerySet
    count: Optional[int]
    big_query: Optional[QuerySet]
    only_list: Set[str]

    def __init__(self, kwargs):
        if 'common_task' in kwargs:
            kwargs['mounting_task__isnull'] = not kwargs.pop('common_task')

        self.additional_filters = []
        additional_related_from_filters = []
        self.additional_only_fields = []

        for (is_id, query_field_name, field_name, table, op) in FILTER_FIELDS_OF_CONSTURCTION:
            transform_construction_filter_value(
                self.additional_filters,
                additional_related_from_filters,
                self.additional_only_fields,
                table,
                field_name,
                op,
                (convert_ID_to_id(kwargs.pop(query_field_name, None)) if is_id else kwargs.pop(query_field_name, None)),
            )
        for (is_id, query_field_name, field_name, table, op) in FILTER_FIELDS_OF_CONSTURCTION_SIDE:
            transform_construction_side_filter_value(
                self.additional_filters,
                additional_related_from_filters,
                self.additional_only_fields,
                table,
                field_name,
                op,
                (convert_ID_to_id(kwargs.pop(query_field_name, None)) if is_id else kwargs.pop(query_field_name, None)),
            )

        self.related_set = set()
        self.fast_filter = []
        project_id = convert_ID_to_id(kwargs.pop('project_id', None))
        if project_id:
            kwargs['reservation__project_id'] = project_id
            self.additional_only_fields.append('reservation__project_id')
            self.additional_only_fields.append('reservation_id')
            self.related_set.add('reservation')

        self.order_by_list, order_by_related_set, self.order_by_annotations = get_order_by_list(
            kwargs, ORDERING_FIELDS, MOBILE_QUERY_ORDERING_ANNOTATION_FIELDS
        )
        self.order_by_list.append('id')

        self.offset, self.limit = get_offset_limit(kwargs, LIMIT)
        self.related_set = self.related_set | set(additional_related_from_filters) | order_by_related_set

        # в список полей only нужно добавлять related_set, иначе будет ошибка вида
        # Field Reservation.construction_side cannot be both deferred and traversed using select_related at the same
        # time.
        self.only_list = set(chain(ONLY_MOUNTING_LIST, self.additional_only_fields, order_by_related_set))
        if self.order_by_list:
            self.only_list.add('reservation__construction_side')
            self.only_list.add('construction_side')

        self.init_fast_search_query(kwargs)
        # print(self.additional_filters)
        # print(self.fast_filter)
        # print(self.related_set)
        # print(self.only_list)
        # print(self.order_by_annotations)
        # print(self.order_by_list)

        self.query = (
            m.Mounting.objects.filter(*self.additional_filters, *self.fast_filter, **kwargs)
            .select_related(
                *self.related_set,
            )
            .only(*sorted(self.only_list))
            .annotate(**self.order_by_annotations)
            .order_by(*self.order_by_list)
        )
        # print(self.additional_filters, self.fast_filter, kwargs)
        # print('---')
        # print(self.order_by_annotations)
        # print('---')
        print(self.order_by_list)
        # print(self.query)

        self.count = None
        self.big_query = None

    def init_fast_search_query(self, kwargs):
        """Создать спецификатор фильтра быстрого поиска"""
        fast_str, fast_int, fast_date = get_fast_search_param(kwargs)
        if fast_str:
            self.related_set |= set(get_construction_side_q_related('advertising_side__side__format')) | set(
                # get_construction_q_related('marketing_address')
            )
            q = construction_side_code_to_filterspec(fast_str, 'reservation__construction_side__')
            iq = (
                Q(crew__name__icontains=fast_str)
                | get_construction_q('marketing_address__address__icontains', fast_str)
                | get_construction_side_q('advertising_side__side__format__title__icontains', fast_str)
                | get_construction_side_q('advertising_side__title__icontains', fast_str)
                | Q(photos__num__icontains=fast_str)
                | Q(comment__icontains=fast_str)
                | Q(construction__marketing_address__address__icontains=fast_str)
                | Q(mounting_task__title__icontains=fast_str)
                | Q(reservation__project__title__icontains=fast_str)
            )
            self.only_list.add('mounting_task__title')
            self.only_list.add('reservation__project__title')

            self.only_list.add('reservation__construction_side__advertising_side')
            self.only_list.add('construction_side__advertising_side')

            self.related_set.add('mounting_task')
            self.related_set.add('reservation__project')

            q = q | iq if q is not None else iq
            if fast_int:
                fast_int_str = str(fast_int)

                if 0 < fast_int < 32:
                    iq = (
                        Q(start_mounting__day=fast_int_str)
                        | Q(end_mounting__day=fast_int_str)
                        | Q(downloaded_early__day=fast_int_str)
                    )
                    q = q | iq if q is not None else iq

                if 0 < fast_int < 13:
                    iq = (
                            Q(start_mounting__month=fast_int_str)
                            | Q(end_mounting__month=fast_int_str)
                            | Q(downloaded_early__month=fast_int_str)
                    )
                    q = q | iq if q is not None else iq

                if 1970 < fast_int < 3000:
                    iq = (
                            Q(start_mounting__year=fast_int_str)
                            | Q(end_mounting__year=fast_int_str)
                            | Q(downloaded_early__year=fast_int_str)
                    )
                    q = q | iq if q is not None else iq

            if fast_date:
                iq = Q(start_mounting=fast_date) | Q(end_mounting=fast_date) | Q(downloaded_early=fast_date)
                q = q | iq if q is not None else iq
            if q is not None:
                self.fast_filter.append(q)

    def mobile_query(self, kwargs):
        if isinstance(self.limit, int) and self.limit == 0:
            ids = [v.id for v in self.query[self.offset:]]
        else:
            ids = [v.id for v in self.query[self.offset : self.offset + self.limit]]

        self.big_query = (
            m.Mounting.objects.filter(id__in=ids)
            .prefetch_related(
                'photos',
                Prefetch('reservation__appendix', m.Appendix.objects.only('id', 'code')),
            )
            .select_related(
                *(
                    self.related_set
                    | {
                        'mounting_task',
                        'reservation',
                        'reservation__project',
                        'construction_side',
                        'construction_side__construction',
                        'construction',
                        'design',
                        'unmounting_design',
                        'crew',
                        'crew__city',
                        *format_related(''),
                        *format_related('reservation__'),
                        *mounting_construction_related(''),
                    }
                ),
            )
            .only(
                *(
                    set(ONLY_MOUNTING_LIST)
                    | set(self.additional_only_fields)
                    | {
                        'design_id',
                        'design__img',
                        'design__started_at',
                        'design__title',
                        'mounting_task__title',
                        'unmounting_design_id',
                        'unmounting_design__img',
                        'unmounting_design__started_at',
                        'unmounting_design__title',
                        'crew__id',
                        'crew__num',
                        'crew__name',
                        'crew__phone',
                        'crew__city__title',
                        'reservation__project__code',
                        'reservation__project__title',
                        'reservation__construction_side_id',
                        'reservation_id',
                        'construction_side_id',
                        *get_mounting_only_fields('reservation__'),
                        *get_mounting_only_fields(''),
                        *get_mounting_construction_fields(''),
                    }
                )
            )
            .annotate(
                **{
                    **MOBILE_QUERY_COMMON_ANNOTATION_FIELDS,
                    **self.order_by_annotations,
                }
            )
            .order_by(*self.order_by_list)
        )

        self.count = m.Mounting.objects.filter(*self.additional_filters, *self.fast_filter, **kwargs).count()


def db_query(kwargs):
    q = DbQueryCommon(kwargs)
    q.mobile_query(kwargs)
    return q.big_query, q.offset, q.limit, q.count, q.order_by_list
