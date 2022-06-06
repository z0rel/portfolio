from typing import List, Tuple, Dict, Set
from django.db.models import Q


TReservationFilter = Dict[str, str]
TReservationFilterList = List[Q]
TRelatedSetReservations = Set[str]


def get_reservation_filters(
    kwargs, related_args_mapped_reservation: Dict[str, str] = None
) -> Tuple[TReservationFilter, TReservationFilterList, TRelatedSetReservations]:
    """
    @param kwargs:
      Ключи фильтрации:
        reservation__date_from__gte: graphql.DateTime - >= заданной даты начала бронирования
        reservation__date_to__lte: graphql.DateTime - <= заданной даты окончания бронирования
    @param related_args_mapped_reservation:
      словарь сопоставлений имя входного ключа фильтра -> имя таблицы добавляемой в select_related
    @return: Кортеж
      0 - словарь Имя поля фильтра: значение фильтра
      1 - список специфиаторов фильтрации
      2 - множество добавляемых к select_related таблиц
    """
    related_set_reservations: TRelatedSetReservations = set()
    reservation_filter: TReservationFilter = {}

    if related_args_mapped_reservation:
        for k in {**kwargs}:
            if k in related_args_mapped_reservation:
                reservation_filter[k] = kwargs.pop(k)
                related_set_reservations.add(related_args_mapped_reservation[k])

    reservation_filter_list: TReservationFilterList = []
    if 'reservation__date_from__gte' in kwargs and 'reservation__date_to__lte' in kwargs:
        query_date_from = kwargs.pop('reservation__date_from__gte')
        query_date_to = kwargs.pop('reservation__date_to__lte')
        reservation_filter_list = [
            # дата начала попадает в заданный в запросе интервал
            Q(date_from__gte=query_date_from, date_from__lte=query_date_to)
            # дата окончания попадает в заданный в запросе интервал
            | Q(date_to__gte=query_date_from, date_to__lte=query_date_to)
        ]
    elif 'reservation__date_from__gte' in kwargs:
        query_date_from = kwargs.pop('reservation__date_from__gte')
        reservation_filter['date_from__gte'] = query_date_from
    if 'reservation__date_to__lte' in kwargs:
        query_date_to = kwargs.pop('reservation__date_to__lte')
        reservation_filter['date_to__lte'] = query_date_to

    return reservation_filter, reservation_filter_list, related_set_reservations
