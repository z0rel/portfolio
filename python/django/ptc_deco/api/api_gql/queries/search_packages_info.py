import graphene
import graphene as g
from graphene import Node
from graphql import GraphQLError
from django.utils import timezone
from django.db.models import Count
from collections import defaultdict
from .optim.generate_order_by_class import graphql__order_by__offset__limit__mixin, get_order_by_list
from .utils.get_offset_limit import get_offset_limit
from .utils.protobuf_utils import PageInfoApi
from ..utils.enum.time_periods import Period, UnitOfTime
from ..utils.db.datetime_filters import PERIOD_FILTERS, PERIOD_FIRST_DAY
from ..utils.ranges_list_generators import gen_days_ranges, gen_weeks_ranges, gen_months_ranges
from ...models import ReservationType
from ....api import models as m


def count_reserved_sides_by_uot(queryset, date_from, date_to, unit_of_time='month'):
    # Считает зарезервированные стороны пакета за период
    # с группировкой по единице времени
    # Возвращает список списков в формате
    # [[[начало_периода, конец_периода], {id_пакета: число сторон}]]
    data = []
    if unit_of_time == 'month':
        ranges = gen_months_ranges(date_from, date_to)
    elif unit_of_time == 'week':
        ranges = gen_weeks_ranges(date_from, date_to)
    elif unit_of_time == 'day':
        ranges = gen_days_ranges(date_from, date_to)
    else:
        raise Exception('Not supported unit_of_time provided.')
    for r in ranges:
        data.append([r, defaultdict(set)])
    for rec in data:
        for record in queryset:
            if rec[0][0] <= record.date_from.date() <= rec[0][1] \
                    or rec[0][0] <= record.date_to.date() <= rec[0][1] \
                    or record.date_from.date() < rec[0][0] and record.date_to.date() > rec[0][1]:
                rec[1][record.construction_side.package.id].add(record.construction_side_id)
        for package_id in rec[1]:
            rec[1][package_id] = len(rec[1][package_id])
    return data


def prepare_packages_info(kwargs, without_sides_values=False):
    packages_ids_global = kwargs.get('packages_ids', [])
    unit_of_time = kwargs.get('unit_of_time', 'month')
    period = kwargs.get('period', 'last_year')
    date_from = kwargs.get('date_from', timezone.now().date())
    date_to = kwargs.get('date_to', timezone.now().date())

    offset, limit = get_offset_limit(kwargs, None)
    order_by, order_by_related_set, order_by_annotations = get_order_by_list(kwargs, ORDERING_FIELDS)

    packages_ids = []
    for pid in packages_ids_global:
        _type, _id = Node.from_global_id(pid)
        packages_ids.append(_id)

    if date_from > date_to:
        raise GraphQLError('Начало диапазона не может быть позднее конца')

    packages = m.Package.objects.filter(
        id__in=packages_ids
    ) if packages_ids else m.Package.objects.all()

    count = packages.count()

    if order_by:
        packages = packages.order_by(*order_by)
    else:
        packages = packages.order_by('id', 'title', 'year', 'month', )

    packages = packages.values(
        'id',
        'city__title',
        'title',
        'year',
        'month',
    )

    if limit:
        packages = packages[offset: offset + limit]
    else:
        packages = packages[offset:]

    sides = m.ConstructionSide.objects.filter(
        package_id__in=packages_ids,
    ).select_related(
        'package',
        'construction__marketing_address',
        'package__city',
        'advertising_side__title',
    )

    sides_values = None
    if not without_sides_values:
        sides_values = sides.values(
            'package_id',
            'construction__marketing_address__address',
            'package__title',
            'package__city__title',
            'advertising_side__title',
            'availability_side',
            'is_archive',
            'id',
        ).order_by(
            'package_id',
            'id',
            'is_archive',
        )

    count_sides_by_packages = sides.values('package_id').annotate(count=Count('id'))

    # Только брони с указанными статусами будут учтены
    reserved_with_statuses = [
        ReservationType.ReservationTypeInteger.SALED,
        ReservationType.ReservationTypeInteger.SETTED,
    ]
    reservations = m.Reservation.objects.filter(
        construction_side__in=sides,
        reservation_type__ikey__in=reserved_with_statuses
    )
    from_field_name = 'date_from'
    to_field_name = 'date_to'

    today = timezone.now().date()
    if period == 'custom':
        reservations = PERIOD_FILTERS['ranges_intersection_by_date_range'](
            reservations, from_field_name, to_field_name, date_from, date_to
        )
    else:
        date_to = today
        date_from = PERIOD_FIRST_DAY[period](date_to)
        reservations = PERIOD_FILTERS['ranges_intersection_by_name'](
            reservations, today, from_field_name, to_field_name, period
        )

    counts = count_reserved_sides_by_uot(reservations, date_from, date_to, unit_of_time)

    return packages, sides_values, count_sides_by_packages, counts, offset, limit, count


class PeriodCountType(g.ObjectType):
    class Meta:
        description = 'Статистика за период'

    period_first_day = g.Date(description='Первый день периода')
    period_last_day = g.Date(description='Последний день периода')
    count = g.Int(description='Число броней')


class PackageObjectType(g.ObjectType):
    class Meta:
        description = 'Пакет'

    id = g.ID(description='Идентификатор пакета')
    title = g.String(description='Наименование пакета')
    city = g.String(description='Город')
    year = g.Int(description='Год')
    month = g.String(description='Месяц')
    sides_count = g.Int(description='Всего сторон в пакете')
    reservation_stats = g.List(PeriodCountType, description='Статистика по промежуткам')


class PackagesInfoQuery(g.ObjectType):
    class Meta:
        description = 'Статистика бронирования по пакетам'

    packages_statistic = g.List(
        PackageObjectType,
    )
    pageInfo = g.Field(PageInfoApi, description='Данные пагинации')


def resolve_packages_statistic(root, info, **kwargs):
    packages, sides_values, count_sides_by_packages, counts, offset, limit, total_count = prepare_packages_info(
        kwargs, without_sides_values=True
    )

    package_objects_list = []

    sides_counts = {}
    for count in count_sides_by_packages:
        package_id = count['package_id']
        count = count['count']
        sides_counts[package_id] = count

    for r in packages:
        package_id = r.get('id', None)

        stats = []
        for s in counts:
            stats.append(
                PeriodCountType(
                    period_first_day=s[0][0],
                    period_last_day=s[0][1],
                    count=s[1].get(package_id, 0)
                )
            )

        package_objects_list.append(
            PackageObjectType(
                id=Node.to_global_id('VPackageNode', package_id),
                title=r.get('title', None),
                city=r.get('city__title', None),
                year=r.get('year', None),
                month=r.get('month', None),
                sides_count=sides_counts.get(package_id, 0),
                reservation_stats=stats,
            )
        )

    return package_objects_list, total_count, offset, limit


def resolve_packages_info(root, info, **kwargs):
    stats, total, offset, limit = resolve_packages_statistic(root, info, **kwargs)
    return PackagesInfoQuery(
        packages_statistic=stats,
        pageInfo=PageInfoApi(total_count=total, offset=offset, limit=limit)
    )


PACKAGE_STATS_FILTER_FIELDS = {
    'packages_ids': g.List(
        g.ID,
        description='Список идентификаторов пакетов',
    ),
    'unit_of_time': UnitOfTime(
        description='Единица времени, по которой производится агрегация'
    ),
    'period': Period(
        description='Период, за который производится выборка. Если указан тип CUSTOM, фильтрует в диапазоне [date_from, date_to]'
    ),
    'date_from': g.Date(
        description='Дата начала периода. Указанный день включается в выборку [date_from:date_to)'
    ),
    'date_to': g.Date(
        description='Дата конца периода. Указанный день не включается в выборку [date_from:date_to)'
    ),
}

ORDERING_FIELDS = {
      'id': (['id'], 'По идентификатору пакета', []),
      'year': (['year'], 'По году', []),
      'month': (['month'], 'По месяцу', []),
      'title': (['title'], 'По названию', []),
}


packages_info_field = graphene.Field(
    PackagesInfoQuery,
    **PACKAGE_STATS_FILTER_FIELDS,
    **graphql__order_by__offset__limit__mixin('PackagesInfoOrderBy', ORDERING_FIELDS),
    description='Статистика по пакетам',
)
