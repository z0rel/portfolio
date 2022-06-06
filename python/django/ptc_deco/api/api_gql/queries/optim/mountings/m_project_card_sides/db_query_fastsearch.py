from datetime import datetime
from typing import Optional

from django.db.models import Q

from ....utils import construction_side_code_to_filterspec
from ...generate_order_by_class import get_fast_search_param


def reduce_q(q, iq):
    if q is not None and iq is not None:
        return q | iq
    elif q is not None:
        return q
    return iq


def init_fast_search_query(kwargs):
    """Создать спецификатор фильтра быстрого поиска"""

    fast_str, fast_int, fast_date = get_fast_search_param(kwargs)
    fast_bool = None
    fast_rts = None

    if fast_str == 'Да':
        fast_bool = True
    elif fast_str == 'Нет':
        fast_bool = False
    elif fast_str == 'РТС':
        fast_rts = True

    # hq = None
    q = None


    if fast_str:
        q = construction_side_code_to_filterspec(fast_str, 'construction_side__', from_reservation=True)
        q = reduce_q(
            q,
            Q(construction_side__construction__postcode__district__city__title__icontains=fast_str)
            | Q(construction_side__construction__marketing_address__address__icontains=fast_str)
            | Q(construction_side__advertising_side__side__format__title__icontains=fast_str)
            | Q(construction_side__advertising_side__side__format__code__icontains=fast_str)
            | Q(construction_side__advertising_side__title__icontains=fast_str)
            | Q(construction_side__advertising_side__side__size__icontains=fast_str)
            | Q(construction_side__construction__crew__name__icontains=fast_str)
            | Q(construction_side__construction__nonrts_owner__title__icontains=fast_str)
        )
        q = reduce_q(q, Q(comments__icontains=fast_str))

        #        | Q(unmounting_design_img__icontains=fast_str)
        #        | Q(previous_design_img__icontains=fast_str)
        #        | Q(current_design_img__icontains=fast_str)

        # if fast_int:
        #     fast_int_str = str(fast_int)
        #     iq = Q(construction_side__construction__num_in_district=fast_int_str)
        #     q = q | iq if q is not None else iq
        if fast_date:
            q = reduce_q(
                q,
                (
                    Q(
                        min_date_mounting__year=fast_date.year,
                        min_date_mounting__month=fast_date.month,
                        min_date_mounting__day=fast_date.day,
                    )
                    | Q(
                        max_date_mounting__year=fast_date.year,
                        max_date_mounting__month=fast_date.month,
                        max_date_mounting__day=fast_date.day,
                    )
                    | Q(
                        min_date_unmounting__year=fast_date.year,
                        min_date_unmounting__month=fast_date.month,
                        min_date_unmounting__day=fast_date.day,
                    )
                ),
            )

        if fast_rts:
            q = reduce_q(q, Q(construction_side__construction__nonrts_owner__isnull=True))

        if fast_bool is not None:
            q = reduce_q(q, Q(construction_side__construction__status_connection=fast_bool))

    return q
