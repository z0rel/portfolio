from datetime import datetime, timedelta
from typing import List

import calendar


class SplittedPeriodItem:
    __slots__ = ('date_from', 'date_to', 'period_days', 'month_from_days')

    def __init__(self, date_from: datetime, date_to: datetime, period_days: int, month_from_days: int):
        self.date_from: datetime = date_from
        self.date_to: datetime = date_to
        self.period_days: int = period_days
        self.month_from_days: int = month_from_days


def get_period_id(date_from: datetime, period_days: int, month_days: int) -> int:
    """
        Получить идентификатор периода по дате начала размещения и числу дней
    """
    if date_from.month == 2:
        second_period_days = 14
    else:
        second_period_days = 16

    if period_days <= 7:
        return 1
    elif period_days <= second_period_days:
        return 2
    elif period_days <= 21:
        return 3
    elif period_days <= month_days:
        return 4
    else:
        return 5


def split_period_by_months(date_from: datetime, date_to: datetime) -> List[SplittedPeriodItem]:
    date_from = datetime(date_from.year, date_from.month, date_from.day)

    if date_to.hour > 0 or date_to.minute > 0:
        date_to = datetime(date_to.year, date_to.month, date_to.day) + timedelta(days=1)
    else:
        date_to = datetime(date_to.year, date_to.month, date_to.day)

    if date_from > date_to:
        return []

    if date_from == date_to:
        return [SplittedPeriodItem(
            date_from,
            date_to,
            1,
            calendar.monthrange(date_from.year, date_from.month)[1],
        )]

    intervals = []
    start_date = date_from
    while True:
        days = calendar.monthrange(start_date.year, start_date.month)[1]
        end_date = start_date + timedelta(days=days)
        if end_date >= date_to:
            intervals.append(SplittedPeriodItem(start_date, date_to, (date_to-start_date).days, days))
            break
        intervals.append(SplittedPeriodItem(start_date, end_date, (end_date-start_date).days, days))
        start_date = end_date

    return intervals


def calc_price(price, periods: dict) -> float:
    """

    @type price: m.PlacementPrice
    """
    summa = price.price_for_placement / periods[price.period]
    return round(summa, 2)
