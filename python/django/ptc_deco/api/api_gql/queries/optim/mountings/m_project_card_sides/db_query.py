from collections import OrderedDict
from itertools import chain
from typing import Union, Dict, Tuple, List, Any

# import sqlparse
from django.contrib.postgres.aggregates import StringAgg, ArrayAgg
from django.db.models import Q, F, Prefetch, Min, Max, Subquery, OuterRef, Count

from .......api import models as m
from ....utils import convert_ID_to_id, get_offset_limit, ModelType, construction_side_code_to_filterspec
from ...generate_order_by_class import get_order_by_list, get_fast_search_param
from ..m_mobile.db_query import (
    ONLY_MOUNTING_LIST,
    MOBILE_QUERY_FIELDS,
    get_query_construction_annotated_fields,
    AnnotationOfQueryConstructionFields,
    AnnotationMobileQueryFields,
    MOBILE_QUERY_COMMON_ANNOTATION_FIELDS,
    get_annotated_construction_param,
)
from .graphql_query_field import RESERVATION_PROJECT_CARD_FILTER_FIELDS
from .db_query_fastsearch import init_fast_search_query
from .order_by import (
    ORDERING_FIELDS,
    ORDERING_PREFETCHED_PHOTO_FIELDS,
    ORDERING_PREFETCHED_MOUNTING_FIELDS,
)

LIMIT = 500


AnnotationReservationMountings = Union[m.Mounting, AnnotationMobileQueryFields]


class AnnotationPrefetchedMountings:
    reservation_mountings: AnnotationReservationMountings
    comments: type(m.Mounting.comment)
    min_date_mounting: type(m.Mounting.start_mounting)
    max_date_mounting: type(m.Mounting.start_mounting)
    min_date_unmounting: type(m.Mounting.end_mounting)
    max_date_unmounting: type(m.Mounting.end_mounting)
    min_photo_date: type(m.MountingPhoto.date)
    min_photo_additional_day_date: type(m.MountingPhoto.date)
    min_photo_additional_night_date: type(m.MountingPhoto.date)
    crews: type(m.Crew.name)
    selected_construction_id: type(m.ConstructionSide.construction_id)
    unmounting_design_id: type(m.Design.id)
    previous_design_id: type(m.Design.id)
    current_design_id: type(m.Design.id)


QUERY_CONSTRUCTION_ANNOTATED_FIELDS = OrderedDict(
    get_query_construction_annotated_fields(
        get_annotated_format_param=lambda x: F('construction_side__advertising_side__side__' + x),
        get_annotated_side_param=lambda x: F('construction_side__' + x),
        get_annotated_construction_param=lambda x: F('construction_side__construction__' + x),
        prefix='',
    )
)

MOUNTINGS_FILTER_FIELDS_START_MOUNTINGS = {
    'start_mounting__contains',
    'start_mounting__gt',
    'start_mounting__gte',
    'start_mounting__lt',
    'start_mounting__lte',
}

MOUNTINGS_FILTER_FIELDS_END_MOUNTINGS = {
    'end_mounting__contains',
    'end_mounting__gt',
    'end_mounting__gte',
    'end_mounting__lt',
    'end_mounting__lte',
}

MOUNTINGS_FILTER_FIELDS = MOUNTINGS_FILTER_FIELDS_START_MOUNTINGS | MOUNTINGS_FILTER_FIELDS_END_MOUNTINGS


FILTER_PHOTO = Q(reservation_mountings__mounting_task_id__isnull=True)
FILTER_PHOTO_ADDITIONAL_DAY = Q(reservation_mountings__mounting_task__title='Дополнительный дневной фотоотчет')
FILTER_PHOTO_ADDITIONAL_NIGHT = Q(reservation_mountings__mounting_task__title='Дополнительный ночной фотоотчет')

ANNOTATION_FIELDS_MOUNTINGS_PROJECT_CARD = [
    ('comments', StringAgg(F('reservation_mountings__comment'), ';', distinct=True)),
    ('min_date_mounting', Min(F('reservation_mountings__start_mounting'), filter=FILTER_PHOTO)),
    ('max_date_mounting', Max(F('reservation_mountings__start_mounting'), filter=FILTER_PHOTO)),
    ('min_date_unmounting', Min(F('reservation_mountings__end_mounting'), filter=FILTER_PHOTO)),
    ('max_date_unmounting', Max(F('reservation_mountings__end_mounting'), filter=FILTER_PHOTO)),
]
ANNOTATION_FIELDS_COMMENTS_D_MOUNTING_PROJECT_CARD = OrderedDict(ANNOTATION_FIELDS_MOUNTINGS_PROJECT_CARD)
ANNOTATION_FIELDS_MOUNTING_PROJECT_CARD = OrderedDict(
    [
        *ANNOTATION_FIELDS_MOUNTINGS_PROJECT_CARD,
        (
            'unmounting_design_id',
            Subquery(
                m.Mounting.objects.filter(
                    reservation_id=OuterRef('id'),
                )
                .order_by('start_mounting', 'id')
                .values('unmounting_design_id')[:1]
            ),
        ),
        (
            'previous_design_id',
            Subquery(
                m.Mounting.objects.filter(
                    reservation_id=OuterRef('id'),
                )
                .order_by('start_mounting', 'id')
                .values('previous_design_id')[:1]
            ),
        ),
        (
            'current_design_id',
            Subquery(
                m.Mounting.objects.filter(
                    reservation_id=OuterRef('id'),
                )
                .order_by('-start_mounting', 'id')
                .values('design_id')[:1]
            ),
        ),
        ('min_photo_date', Min(F('reservation_mountings__photos__date'), filter=FILTER_PHOTO)),
        (
            'min_photo_additional_day_date',
            Min(F('reservation_mountings__photos__date'), filter=FILTER_PHOTO_ADDITIONAL_DAY),
        ),
        (
            'min_photo_additional_night_date',
            Min(F('reservation_mountings__photos__date'), filter=FILTER_PHOTO_ADDITIONAL_NIGHT),
        ),
        ('crews', StringAgg(F('reservation_mountings__crew__name'), ',', distinct=True)),
        ('selected_construction_id', F('construction_side__construction_id')),
        *QUERY_CONSTRUCTION_ANNOTATED_FIELDS.items(),
    ],
)

MOUNTING_FILTER_STATIC = (
    Q(mounting_task__isnull=True)
    | Q(mounting_task__title='Дополнительный ночной фотоотчет')
    | Q(mounting_task__title='Дополнительный дневной фотоотчет')
)


def db_query(
    kwargs: Dict[str, str]
) -> Tuple[
    ModelType[Union[m.Reservation, AnnotationPrefetchedMountings, AnnotationOfQueryConstructionFields]],
    int,
    int,
    int,
    Dict[str, Any],
]:
    # В проекте есть стороны (бронирования), нужно выбрать их
    # затем выбрать все монтажные задачи, связанные с заданным бронированием

    # выбрать все монтажи согласно фильтрационным настройкам и настройкам проекта.
    # т.е. выбрать всё, что не является ремонтно-монтажной задачей - mounting_task = Null
    # Нужно в задачу по монтажу добавить список дополнительных дневных и ночных фотоотчетов
    project_id = convert_ID_to_id(kwargs['project_id'])
    offset, limit = get_offset_limit(kwargs, LIMIT)
    fast_filter_query = init_fast_search_query(kwargs)

    order_by_list, order_by_related_set, order_by_annotations = get_order_by_list(
        kwargs, ORDERING_FIELDS, ANNOTATION_FIELDS_MOUNTING_PROJECT_CARD
    )
    order_by_list.append('id')

    len_reservation_key = len('reservation__')
    reservation_filterspec = {
        k[len_reservation_key:]: v for (k, v) in kwargs.items() if k in RESERVATION_PROJECT_CARD_FILTER_FIELDS
    }
    mounting_filterspec = {k: v for (k, v) in kwargs.items() if k in MOUNTINGS_FILTER_FIELDS}
    for k, v in kwargs.items():
        if k in MOUNTINGS_FILTER_FIELDS:
            mounting_filterspec[k] = v
        if k in MOUNTINGS_FILTER_FIELDS_START_MOUNTINGS:
            mounting_filterspec['start_mounting__isnull'] = False
        if k in MOUNTINGS_FILTER_FIELDS_END_MOUNTINGS:
            mounting_filterspec['end_mounting__isnull'] = False

    # Выбрать все бронирования проекта
    reservations_ids_unordered = m.Reservation.objects.filter(project_id=project_id, **reservation_filterspec)

    if mounting_filterspec:
        reservations_ids_unordered = (
            reservations_ids_unordered.prefetch_related(
                Prefetch(
                    'reservation_mountings', m.Mounting.objects.filter(MOUNTING_FILTER_STATIC, **mounting_filterspec)
                )
            )
            .annotate(count_reservation_mountings=Count('reservation_mountings'))
            .filter(count_reservation_mountings__gt=0)
        )

    if fast_filter_query is None:
        reservations_ids = (
            reservations_ids_unordered.only('id', 'project_id')
            .select_related(*order_by_related_set)
            .annotate(**order_by_annotations)
            .order_by(*order_by_list)
        )
    else:
        reservations_ids_unordered = reservations_ids_unordered.annotate(
            **ANNOTATION_FIELDS_COMMENTS_D_MOUNTING_PROJECT_CARD
        ).filter(fast_filter_query)

        # print(sqlparse.format(str(reservations_ids_unordered.query), reindent=True, keyword_case='upper'))

        reservations_ids = (
            reservations_ids_unordered.select_related(
                'construction_side__construction__location__postcode__district__city',
                'construction_side__construction__location__marketing_address',
                'construction_side__construction__location__legal_address',
                'construction_side__construction__nonrts_owner',
                'construction_side__advertising_side__side__format__model__underfamily__family',
                'project',
                *order_by_related_set,
            )
            .annotate(
                **ANNOTATION_FIELDS_MOUNTING_PROJECT_CARD,
                **{k: v for (k, v) in order_by_annotations.items() if k not in ANNOTATION_FIELDS_MOUNTING_PROJECT_CARD},
            )
            .order_by(*order_by_list)
        )

    count = reservations_ids_unordered.count()

    if offset is not None and limit is not None:
        reservations_ids = reservations_ids[offset : offset + limit]

    m_order_by_list, m_order_by_related_set, m_order_by_annotations = get_order_by_list(
        kwargs, ORDERING_PREFETCHED_MOUNTING_FIELDS
    )
    p_order_by_list, p_order_by_related_set, p_order_by_annotations = get_order_by_list(
        kwargs, ORDERING_PREFETCHED_PHOTO_FIELDS
    )
    reservations_ids = [r.id for r in reservations_ids]

    reservations = (
        m.Reservation.objects.filter(id__in=reservations_ids)
        .prefetch_related(
            Prefetch(
                'reservation_mountings',
                m.Mounting.objects.filter(MOUNTING_FILTER_STATIC, **mounting_filterspec)
                .prefetch_related(
                    Prefetch(
                        'photos',
                        m.MountingPhoto.objects.only('photo', 'date', 'num', 'mounting_id').order_by(
                            *p_order_by_list, 'id'
                        ),
                    )
                )
                .select_related(
                    'design',
                    'unmounting_design',
                    'crew',
                    'crew__city',
                    *m_order_by_related_set,
                )
                .only(
                    *ONLY_MOUNTING_LIST,
                    'design__img',
                    'design__started_at',
                    'design__title',
                    'mounting_task__title',
                    'unmounting_design__img',
                    'unmounting_design__started_at',
                    'unmounting_design__title',
                    'crew__id',
                    'crew__num',
                    'crew__name',
                    'crew__phone',
                    'crew__city__title',
                    'reservation_id',
                )
                .annotate(**MOBILE_QUERY_FIELDS, **m_order_by_annotations)
                .order_by(*m_order_by_list, 'id'),
            )
        )
        .select_related(
            'construction_side__construction__location__postcode__district__city',
            'construction_side__construction__location__marketing_address',
            'construction_side__construction__location__legal_address',
            'construction_side__construction__nonrts_owner',
            'construction_side__advertising_side__side__format__model__underfamily__family',
            'project',
            *order_by_related_set,
        )
        .annotate(
            **ANNOTATION_FIELDS_MOUNTING_PROJECT_CARD,
            **{k: v for (k, v) in order_by_annotations.items() if k not in ANNOTATION_FIELDS_MOUNTING_PROJECT_CARD},
        )
        .order_by(*order_by_list)
    )

    # 1 бронирование - 1 монтаж, 1 демонтаж + какое-то количество дневных и ночных фотоотчетов

    return reservations, offset, limit, count, mounting_filterspec
