from django.db.models import Q
from ...generate_order_by_class import get_fast_search_param


def init_fast_search_query(kwargs):
    """Создать спецификатор фильтра быстрого поиска"""

    fast_str, fast_int, fast_date = get_fast_search_param(kwargs)

    q = Q()
    if fast_str:
        q |= Q(
                Q(advertising_side__side__format__title__icontains=fast_str) |
                Q(construction__location__postcode__district__city__title__icontains=fast_str, ) |
                Q(advertising_side__side__code__icontains=fast_str) |
                Q(advertising_side__code__icontains=fast_str) |
                Q(advertising_side__side__format__code__icontains=fast_str)
            )

    return q

