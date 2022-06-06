import datetime
from dateutil.relativedelta import relativedelta
from django.db.models import Q
from django.utils import timezone


def current_week_first_day(today):
    return today - datetime.timedelta(days=datetime.datetime.today().weekday() % 7)


def current_month_first_day(today):
    return today.replace(day=1)


def current_quarter_first_day(today):
    quarter = (today.month - 1) / 3 + 1
    return datetime.datetime(year=today.year, month=int(3 * quarter - 2), day=1)


def current_year_first_day(today):
    return today.replace(day=1, month=1)


def last_week_first_day(today):
    return today - relativedelta(weeks=1)


def last_month_first_day(today):
    return today - relativedelta(months=1)


def last_quarter_first_day(today):
    return today - relativedelta(months=3)


def last_year_first_day(today):
    return today - relativedelta(years=1)


PERIOD_FIRST_DAY = {
    'current_week': current_week_first_day,
    'current_month': current_month_first_day,
    'current_quarter': current_quarter_first_day,
    'current_year': current_year_first_day,
    'last_week': last_week_first_day,
    'last_month': last_month_first_day,
    'last_quarter': last_quarter_first_day,
    'last_year': last_year_first_day,
}


def filter_created_by_day(queryset, datetime_field_name):
    return queryset.filter(
        **{
            f'{datetime_field_name}__hour__gte': 9,
            f'{datetime_field_name}__hour__lte': 18
        }
    )


def filter_created_at_night(queryset, datetime_field_name):
    return queryset.exclude(
        **{
            f'{datetime_field_name}__hour__gte': 9,
            f'{datetime_field_name}__hour__lte': 18
        }
    )


TIME_OF_DAY_FILTERS = {
    'day': filter_created_by_day,
    'night': filter_created_at_night,
}


def filter_by_period_name(queryset, today, datetime_field_name, period_name, **kwargs):
    return queryset.filter(
        **{
            f'{datetime_field_name}__gte': PERIOD_FIRST_DAY[period_name](today),
            f'{datetime_field_name}__lte': today
        }
    )


def filter_by_date_range(queryset, today, datetime_field_name, **kwargs):
    date_from = kwargs.get('date_from', None)
    date_to = kwargs.get('date_to', None)
    min_time = datetime.datetime.min.time()
    return queryset.filter(
        **{
            f'{datetime_field_name}__gte': datetime.datetime.combine(date_from, min_time) if date_from else today,
            f'{datetime_field_name}__lte': datetime.datetime.combine(date_to, min_time) if date_to else today
        }
    )


def filter_ranges_intersection_by_name(queryset, today, from_field_name, to_field_name, period_name):
    period_first_day = PERIOD_FIRST_DAY[period_name](today)
    return filter_intersections(queryset, from_field_name, to_field_name, period_first_day, today)


def filter_ranges_intersection_by_date_range(queryset, from_field_name, to_field_name, date_from, date_to):
    date_from = date_from if date_from else timezone.now()
    date_to = date_to if date_to else date_from
    return filter_intersections(queryset, from_field_name, to_field_name, date_from, date_to)


def filter_intersections(queryset, from_field_name, to_field_name, date_from, date_to):
    return queryset.filter(
        Q(
            **{
                f'{from_field_name}__lte': date_from,
                f'{to_field_name}__gte': date_from
            }
        ) |
        Q(
            **{
                f'{from_field_name}__gte': date_from,
                f'{to_field_name}__lte': date_to
            }
        ) |
        Q(
            **{
                f'{from_field_name}__lte': date_to,
                f'{to_field_name}__gte': date_to
            }
        ) |
        Q(
            **{
                f'{from_field_name}__lte': date_from,
                f'{to_field_name}__gte': date_to
            }
        )
    )


PERIOD_FILTERS = {
    'by_date_range': filter_by_date_range,
    'by_period_name': filter_by_period_name,
    'ranges_intersection_by_name': filter_ranges_intersection_by_name,
    'ranges_intersection_by_date_range': filter_ranges_intersection_by_date_range,
}
