from django.db.models import Q

from ...generate_order_by_class import get_fast_search_param


def init_fast_search_query(kwargs):
    """Создать спецификатор фильтра быстрого поиска"""

    fast_str, fast_int, fast_date = get_fast_search_param(kwargs)

    q = Q()

    if fast_str:
        q = Q(
            Q(code__icontains=fast_str) |
            Q(title__icontains=fast_str) |
            Q(brand__title__icontains=fast_str) |
            Q(agency__title__icontains=fast_str) |
            Q(project_cities__city__title__icontains=fast_str) |
            Q(client__title__icontains=fast_str) |
            Q(brand__working_sector__title__icontains=fast_str) |
            Q(back_office_manager__name__icontains=fast_str) |
            Q(sales_manager__name__icontains=fast_str)
        )
    if fast_date:
        q |= Q(
            Q(created_at__year=fast_date.year) |
            Q(created_at__month=fast_date.month) |
            Q(created_at__day=fast_date.day)
        )

    return q
