from django.db.models import Count
from django.db.models.functions import TruncMonth, TruncWeek, TruncDay


def count_by_day(queryset, aggregating_field_name):
    return queryset.annotate(day=TruncDay(aggregating_field_name)).values('day').annotate(count=Count('id'))


def count_by_week(queryset, aggregating_field_name):
    return queryset.annotate(week=TruncWeek(aggregating_field_name)).values('week').annotate(count=Count('id'))


def count_by_month(queryset, aggregating_field_name):
    return queryset.annotate(month=TruncMonth(aggregating_field_name)).values('month').annotate(count=Count('id'))


UNIT_OF_TIME_EXACT_COUNTERS = {
    'day': count_by_day,
    'week': count_by_week,
    'month': count_by_month,
}


def count_range_by_month(queryset, date_from_field_name, date_to_field_name):
    return queryset.annotate(
        date_from_month=TruncMonth(date_from_field_name),
        date_to_month=TruncMonth(date_to_field_name)
    ).values(
        'date_from_month',
        'date_to_month'
    ).annotate(
        count_from_intersect=Count('id'),
        count_to_intersect=Count('id')
    )


UNIT_OF_TIME_PERIOD_COUNTERS = {
    'month': count_range_by_month
}
