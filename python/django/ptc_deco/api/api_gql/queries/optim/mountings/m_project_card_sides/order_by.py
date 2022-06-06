from collections import OrderedDict

from django.contrib.postgres.aggregates import StringAgg
from django.db.models import Q, F, Case, When, Prefetch, Min
from ...generate_order_by_class import generate_order_by_class


def field_in_construction(field=None):
    return [
        'reservation__construction_side__construction' + f'__{field}' if field else '',
        'construction_side__construction' + f'__{field}' if field else '',
        'construction' + f'__{field}' if field else '',
    ]


def field_in_construction_side(field):
    return [
        'reservation__construction_side__' + field,
        'construction_side__' + field,
    ]


# 0 - поля сортировки
# 1 - описание
# 2 - related таблицы
# 3 - аннотации

_ORDERING_CONSTRUCTION_FIELDS = {
    'construction_side_code': (
        ['postcode_title', 'num_in_district', 'side_code', 'advertising_side_code'],
        'По коду стороны',
    ),
    'city_title': (['city_title'], 'По городу'),
    'address_marketing': (['address_marketing'], 'По маркетинговому адресу'),
    'address_legal': (['address_legal'], 'По юридическому адресу'),
    'format_title': (['format_title'], 'По формату'),
    'advertising_side_title': (['advertising_side_title'], 'По рекламной стороне'),
    'side_size': (['side_size'], 'По размеру формата'),
    'status_connection': (['status_connection'], 'По наличию освещения'),
    'rts_nonrts': (['nonrts_owner'], 'РТС/НОН РТС'),
    'reservation__date_from': (['date_from'], 'По дате начала бронирования'),
    'reservation__date_to': (['date_from'], 'По дате окончания бронирования'),
    'reservation__branding': (['branding'], 'По факту наличия брендинга'),
}


_ORDERING_MOUNTING_ANNOTATED_FIELDS = {
    'comments': (['comments'], 'По комментариям'),
    'date_mounting': (['min_date_mounting'], 'По дате монтажа'),
    'date_unmounting': (['min_date_unmounting'], 'По дате демонтажа'),
    'photo_images_date': (['min_photo_date'], 'По дате фотоотчета'),
    'photo_additional_day_date': (['min_photo_additional_day_date'], 'По дате дневного доп. фотоотчета'),
    'crews': (['crews'], 'По экипажу'),
    'photo_additional_night_date': (['min_photo_additional_night_date'], 'По дате ночного доп. фотоотчета'),
    'design_filename': (
        [], 'По названию файла дизайна', [],
        [('sorter_design_fname_spec', StringAgg(F('reservation_mountings__design__img'), ','))],
    ),
    'previous_design_filename': (
        [], 'По названию файла предыдущего дизайна', [],
        [('sorter_design_fname_spec', StringAgg(F('reservation_mountings__unmounting_design__img'), ','))],
    ),
}


ORDER_BY_PROJECT_CARD_SIDES_ANNOTATION_FIELDS = OrderedDict(
    [
        ('city_title', F('construction_side__construction__location__postcode__district__city__title')),
    ]
)

_FILTER_PHOTO = Q(reservation_mountings__mounting_task__isnull=True)


def _filter_photo(prefix):
    return {'filter': _FILTER_PHOTO, 'ordering': F(f'{prefix}photos__photo')}


_FILTER_PHOTO_DAY = Q(reservation_mountings__mounting_task__title='Дополнительный дневной фотоотчет')


def _filter_photo_day(prefix):
    return {'filter': _FILTER_PHOTO_DAY, 'ordering': F(f'{prefix}photos__photo')}


_FILTER_PHOTO_NIGHT = Q(reservation_mountings__mounting_task__title='Дополнительный ночной фотоотчет')


def _filter_photo_night(prefix):
    return {'filter': _FILTER_PHOTO_NIGHT, 'ordering': F(f'{prefix}photos__photo')}


def _ordering_mounting_annotation_generated(prefix):
    return {
        'photo_images_filename': (
            [], 'По имени файла фотоотчета', [],
            [('sorter_photo_fname', StringAgg(F(f'{prefix}photos__photo'), ',', **_filter_photo(prefix)))],
        ),
        'photo_additional_day': (
            [], 'По имени файла дополнительного дневного фотоотчета', [],
            [('sorter_a_photo_day', StringAgg(F(f'{prefix}photos__photo'), ',', **_filter_photo_day(prefix)))],
        ),
        'photo_additional_night': (
            [], 'По имени файла дополнительного ночного фотоотчета', [],
            [('sorter_a_photo_night', StringAgg(F(f'{prefix}photos__photo'), ',', **_filter_photo_night(prefix)))],
        ),
    }


ORDERING_PREFETCHED_MOUNTING_FIELDS = {
    'comments': (['comment'], 'По комментариям'),
    'date_mounting': (['start_mounting'], 'По дате монтажа'),
    'date_unmounting': (['end_mounting'], 'По дате демонтажа'),
    'crews': (['crew__name'], 'По экипажу'),
    'design_filename': (['design__img'], 'По названию файла дизайна'),
    'previous_design_filename': (['unmounting_design__img']),
    'photo_images_date': (
        [], 'По дате фотоотчета', [],
        [('min_photo_date', Min(F('photos__date'), filter=_FILTER_PHOTO))],
    ),
    'photo_additional_day_date': (
        [], 'По дате доп. дневного фотоотчета', [],
        [('min_photo_additional_day_date', Min(F('photos__date'), filter=_FILTER_PHOTO_DAY))],
    ),
    'photo_additional_night_date': (
        [], 'По дате доп. ночного фотоотчета', [],
        [('min_photo_additional_night_date', Min(F('photos__date'), filter=_FILTER_PHOTO_NIGHT))],
    ),
    **_ordering_mounting_annotation_generated(''),
}


ORDERING_PREFETCHED_PHOTO_FIELDS = {
    'photo_images_filename': (['photo'], 'По имени файла'),
    'photo_additional_day': (['photo'], 'По имени файла'),
    'photo_additional_night': (['photo'], 'По имени файла'),
    'photo_images_date': (['date'], ''),
    'photo_additional_day_date': (['date'], ''),
    'photo_additional_night_date': (['date'], ''),
}


ORDERING_FIELDS = {
    **_ORDERING_CONSTRUCTION_FIELDS,
    **_ORDERING_MOUNTING_ANNOTATED_FIELDS,
    **_ordering_mounting_annotation_generated('reservation_mountings__'),
}

GraphqlOrderBy = generate_order_by_class('MountingProjectCardOrderBy', ORDERING_FIELDS)
