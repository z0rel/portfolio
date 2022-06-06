from graphene.utils.str_converters import to_snake_case
from django.db.models import F, Prefetch, Q

from ...generate_order_by_class import get_order_by_list
from .......api import models as m
from ....utils import unpack_ids, construct_related_and_only_lists, get_offset_limit
from .get_reservation_filters import get_reservation_filters
from .order_by import ORDERING_FIELDS
from .db_query_fastsearch import init_fast_search_query
from ....utils.create_multiple_codes_filter import create_filter

from .db_mapped_fields import (
    IDS_TO_UNPACK,
    RELATED_ARGS_MAPPED_CONSTRUCTION_SIDE,
    RELATED_ARGS_MAPPED_RESERVATION,
    RELATED_SET_CONSTRUCTION_SIDES,
    RELATED_SET_RESERVATION,
    ONLY_FIELDS_RESERVATION,
    ONLY_FIELDS_CONSTRUCTION,
    CONSTRUCTION_SIDE_ANNOTATION_FIELDS,
    RESERVATION_ANNOTATION_FIELDS,
)


LIMIT = 200


def db_query(kwargs):
    q_codes = Q()
    if 'codes' in kwargs:
        q_codes = create_filter(q_codes, kwargs['codes'])
        del kwargs['codes']

    unpack_ids(kwargs, IDS_TO_UNPACK)

    reservation_filter, reservation_filter_list, related_set_reservations = get_reservation_filters(
        kwargs, RELATED_ARGS_MAPPED_RESERVATION
    )

    order_by_list, order_related_set, order_annotations = get_order_by_list(kwargs, ORDERING_FIELDS)

    local_only_fields_construction, related_list_construction_sides = construct_related_and_only_lists(
        kwargs, ONLY_FIELDS_CONSTRUCTION, RELATED_SET_CONSTRUCTION_SIDES, RELATED_ARGS_MAPPED_CONSTRUCTION_SIDE
    )

    offset, limit = get_offset_limit(kwargs, LIMIT)
    count = None

    fast_filter_query = init_fast_search_query(kwargs)
    filter_expr = Q(Q(**kwargs) & q_codes & fast_filter_query)

    if not not reservation_filter and not reservation_filter_list:
        count = m.ConstructionSide.objects.filter(filter_expr).select_related(*related_list_construction_sides).count()

    related_set_reservations |= RELATED_SET_RESERVATION

    construction_sides = (
        m.ConstructionSide.objects.filter(filter_expr)
        .select_related(*RELATED_SET_CONSTRUCTION_SIDES)
        .prefetch_related(
            Prefetch(
                'reservation',
                queryset=(
                    m.Reservation.objects.filter(*reservation_filter_list, **reservation_filter)
                    .select_related(*related_set_reservations)
                    .only(*ONLY_FIELDS_RESERVATION)
                    .annotate(**RESERVATION_ANNOTATION_FIELDS)
                ),
            )
        )
        .only(*local_only_fields_construction)
        .annotate(**CONSTRUCTION_SIDE_ANNOTATION_FIELDS)
    )
    if count is None:
        count = construction_sides.count()

    if order_by_list:
        construction_sides = construction_sides.order_by(*order_by_list)
    return construction_sides, offset, limit, count
