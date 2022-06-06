from graphene.relay import Node

from ..estimate.graphql_resolvers import (
    MAPPED_FIELDS_ADDITIONAL_NONRTS_NOSIDES_STR,
    MAPPED_FIELDS_ADDITIONAL_NONRTS_NOSIDES_DECIMAL,
    MAPPED_FIELDS_RESOLVE_ADDITIONAL_RTS_ITOGS_STR,
    MAPPED_FIELDS_RESOLVE_ADDITIONAL_RTS_ITOGS_DECIMAL,
    get_resolve_additional_rts_itogs_discount,
    ResolveAdditionalCostsRtsCopyFieldsOptimNewItem,
    resolve_additional_costs_rts_copy_fields,
    MAPPED_FIELDS_RESOLVE_ESTIMATE_NO_SIDES_NONRTS_STR,
    MAPPED_FIELDS_RESOLVE_ESTIMATE_NO_SIDES_NONRTS_DECIMAL,
    resolve_estimate_no_sides_nonrts_ak_zero_predicate,
    MAPPED_FIELDS_RESOLVE_ESTIMATE_NO_SIDES_NONRTS_NONRTS_SECTION_PART_DECIMAL,
    MAPPED_FIELDS_RESOLVE_ADDITIONAL_NONRTS_RESERVATIONS_SALE_PAY_MARGIN,
)
from ....utils.protobuf_utils import set_arg_conv, PageInfoApi, set_arg_getattr, set_arg_conv_getattr, base64_bytes
from ......estimate import EstimateCalc

from ....proto import estimate_pb2
from ....utils import (
    set_arg,
    ContentFieldConnection,
    transform_kwargs_by_mapped_spec,
)
from ..estimate.db_query import get_estimate_obj_by_filter_args
from .graphql_query_field import MAPPED_KWARGS_ESTIMATE_PROTO


def tr_isoformat(v):
    return v.isoformat()


def tr_str(v):
    return str(v)


def map_fields_dict(db_item, pb_item, mapped_str, mapped_decimal, mapped_conv=None):
    if mapped_str:
        for dst, src in mapped_str.items():
            set_arg(pb_item, dst, db_item[src])
    if mapped_decimal:
        for dst, src in mapped_decimal.items():
            set_arg_conv(pb_item, dst, db_item[src], tr_str)
    if mapped_conv:
        for dst, (src, conv) in mapped_conv.items():
            set_arg_conv(pb_item, dst, db_item[src], conv)


def map_fields(db_item, pb_item, mapped_str, mapped_decimal, mapped_conv=None):
    if mapped_str:
        for dst, src in mapped_str.items():
            set_arg(pb_item, dst, getattr(db_item, src))
    if mapped_decimal:
        for dst, src in mapped_decimal.items():
            set_arg_conv(pb_item, dst, getattr(db_item, src), tr_str)
    if mapped_conv:
        for dst, (src, conv) in mapped_conv.items():
            set_arg_conv(pb_item, dst, getattr(db_item, src), conv)


MAPPED_ADDITIONAL_RTS_NOCONV = ['title', 'city_title', 'count', 'category']


def city_id_to_relay_id(v):
    return Node.to_global_id('VCityOptimizedNode', v)


MAPPED_ADDITIONAL_RTS_CONV = [
    ('id', lambda v: Node.to_global_id('VEstimateAdditionalCostsRtsNode', v)),
    ('city_id', lambda v: Node.to_global_id('VCityOptimizedNode', v)),
    ('start_period', tr_isoformat),
    ('end_period', tr_isoformat),
    ('price', tr_str),
]

MAPPED_ADDITIONAL_RTS_CONV_OBJ = [
    'summa_before_discount',
    'discount_value',
    'agency_commission_value',
    'price_after_discount',
    'discount_percent',
    'agency_commission_percent',
    'value_without_agency_commission',
    'summa_after_discount',
]

MAPPED_ESTIMATE_NON_RTS_NOCONV = {'count': 'count'}


MAPPED_ESTIMATE_NON_RTS_CONV = {
    'id': ('id', lambda v: Node.to_global_id('VEstimateNonRtsOptimizedNode', v)),
    'start_period': ('start_period', tr_isoformat),
    'end_period': ('end_period', tr_isoformat),
}


ESTIMATE_NON_RTS_DECIMAL_FIELDS_COMMON = [
    'incoming_rent',
    'incoming_tax',
    'incoming_printing',
    'incoming_manufacturing',
    'incoming_installation',
    'incoming_additional',
    'sale_rent',
    'sale_tax',
    'sale_printing',
    'sale_manufacturing',
    'sale_installation',
    'sale_additional',
]


MAPPED_ESTIMATE_NON_RTS_DECIMAL = {
    **{k: k for k in ESTIMATE_NON_RTS_DECIMAL_FIELDS_COMMON},
    **MAPPED_FIELDS_RESOLVE_ESTIMATE_NO_SIDES_NONRTS_NONRTS_SECTION_PART_DECIMAL,  # sale, pay, margin
}


MAPPED_ESTIMATE_NON_RTS_DECIMAL_RESERVATION = {
    **{k: k for k in ESTIMATE_NON_RTS_DECIMAL_FIELDS_COMMON},
    # **MAPPED_FIELDS_RESOLVE_ADDITIONAL_NONRTS_RESERVATIONS_SALE_PAY_MARGIN,  # sale, pay, margin
}


def set_agency_comission(pb_ak, db_ak):
    if not db_ak:
        return

    set_arg_getattr(pb_ak, 'to_rent', db_ak)
    set_arg_getattr(pb_ak, 'to_nalog', db_ak)
    set_arg_getattr(pb_ak, 'to_print', db_ak)
    set_arg_getattr(pb_ak, 'to_mount', db_ak)
    set_arg_getattr(pb_ak, 'to_additional', db_ak)
    set_arg_getattr(pb_ak, 'to_nonrts', db_ak)

    set_arg_conv_getattr(pb_ak, 'percent', db_ak, tr_str)
    set_arg_conv_getattr(pb_ak, 'value', db_ak, tr_str)


def reservation_id_to_relay_id(v):
    return Node.to_global_id('VReservationOptimizedNode', v)


def reservation_type_id_to_relay_id(v):
    return Node.to_global_id('VReservationTypeOptimizedNode', v)


def construction_side_id_to_relay_id(v):
    return Node.to_global_id('VConstructionSideOptimizedNode', v)


def format_id_to_relay_id(v):
    return Node.to_global_id('VFormatOptimizedNode', v)


def populate_reservation(pb_r, db_r):
    cs = pb_r.construction_side

    set_arg_conv_getattr(pb_r, 'id', db_r, reservation_id_to_relay_id)
    set_arg_conv_getattr(pb_r, 'date_from', db_r, tr_isoformat)
    set_arg_conv_getattr(pb_r, 'date_to', db_r, tr_isoformat)
    set_arg_getattr(pb_r, 'branding', db_r)
    set_agency_comission(pb_r.agency_commission, db_r.agency_commission)
    set_arg_conv_getattr(cs, 'city_id', db_r, city_id_to_relay_id)
    set_arg_getattr(cs, 'city_title', db_r)
    set_arg_conv_getattr(cs, 'format_id', db_r, format_id_to_relay_id)
    set_arg_getattr(cs, 'format_title', db_r)
    set_arg_getattr(cs, 'address_title', db_r)

    set_arg_conv_getattr(pb_r, 'reservation_type_id', db_r, reservation_type_id_to_relay_id)
    set_arg_getattr(pb_r, 'reservation_type_title', db_r)
    set_arg_conv(cs, 'id', getattr(db_r, 'construction_side_id'), construction_side_id_to_relay_id)
    set_arg_getattr(cs, 'postcode_title', db_r)
    set_arg_getattr(cs, 'num_in_district', db_r)
    set_arg_getattr(cs, 'side_title', db_r)
    set_arg_getattr(cs, 'side_code', db_r)
    set_arg_getattr(cs, 'adv_side_title', db_r)
    set_arg_getattr(cs, 'adv_side_code', db_r)
    set_arg_getattr(cs, 'format_code', db_r)
    set_arg_getattr(cs, 'is_nonrts', db_r)
    set_arg_getattr(cs, 'nonrts_owner', db_r)
    set_arg_getattr(cs, 'status_connection', db_r)


MAPPED_FIELDS_CALCULATED_RESERVATION = {
    k: k
    for k in [
        'discount_price_percent_setted',
        'rent_by_price_after_discount_setted',
        'rent_to_client_setted',
        'discount_to_client_percent_setted',
        'rent_to_client_after_discount_setted',
        'discount_nalog_percent_setted',
        'nalog_after_discount_setted',
        'rent_by_price_calculated',
        'discount_price_percent_calculated',
        'discount_price_percent_selected',
        'value_after_discount_price_calculated',
        'value_after_discount_price_selected',
        'value_rent_to_client_selected',
        'discount_client_percent_selected',
        'value_rent_to_client_after_discount_selected',
        'additional_static_printing',
        'additional_static_mounting',
        'additional_static_additional',
        'additional_static_nalog',
        'additional_static_nalog_discount_percent_selected',
        'additional_static_nalog_discount_calculated',
        'additional_static_nalog_value_after_discount',
        'additional_static_nalog_discount_selected',
        'itog_summary',
        'itog_agency_commission',
        'itog_summary_without_agency_commission',
    ]
}

ESTIMATE_SUMMARY_IGNORE_FIELDS = {
    'estimatecalculateditogsapi',
    'project',
    'project_id',
    'appendix',
    'appendix_id',
    'static_additional_details',
    'branding_cost'
}


def estimate_summary_tr(k, x):
    return x if x is None else str(x)


def resolve_estimate(parent, info, **kwargs):
    transform_kwargs_by_mapped_spec(MAPPED_KWARGS_ESTIMATE_PROTO, kwargs, kwargs)
    if not kwargs:
        return ContentFieldConnection(content=None, pageInfo=PageInfoApi(total_count=0, offset=0, limit=100))

    constructor_args, calc_args = get_estimate_obj_by_filter_args(**kwargs)
    e: EstimateCalc = EstimateCalc(**constructor_args)
    e.full_calc(**calc_args)

    result_estimate = estimate_pb2.Estimate()

    # Присвоить основные поля
    e.assign_to_dst_slots_tr(
        e.itog,
        result_estimate.itogs,
        estimate_summary_tr,
        ESTIMATE_SUMMARY_IGNORE_FIELDS,
    )

    result_estimate.errors.extend(sorted(e.estimate_errors))

    # Суммы по дополнительным расходам НОН РТС в разрезе Наименования
    # не включая забронированные рекламные стороны НОН РТС
    for additional_costs_nonrts_by_title_item in e.additional_costs_nonrts_by_title:
        map_fields_dict(
            additional_costs_nonrts_by_title_item,
            result_estimate.additional_nonrts_by_title.add(),
            MAPPED_FIELDS_ADDITIONAL_NONRTS_NOSIDES_STR,
            MAPPED_FIELDS_ADDITIONAL_NONRTS_NOSIDES_DECIMAL,
        )

    # additional_rts_by_title - Суммы по дополнительным расходам РТС в разрезе наименования
    for additional_rts_by_title_item in e.additional_costs_by_title:
        pb_item_additional_rts_by_title = result_estimate.additional_rts_by_title.add()
        map_fields_dict(
            additional_rts_by_title_item,
            pb_item_additional_rts_by_title,
            MAPPED_FIELDS_RESOLVE_ADDITIONAL_RTS_ITOGS_STR,
            MAPPED_FIELDS_RESOLVE_ADDITIONAL_RTS_ITOGS_DECIMAL,
        )
        discount_percent, summa_after_discount = get_resolve_additional_rts_itogs_discount(
            additional_rts_by_title_item['discount_value'],
            additional_rts_by_title_item['summa_before_discount'],
        )
        set_arg_conv(pb_item_additional_rts_by_title, 'discount_percent', discount_percent, tr_str)
        set_arg_conv(pb_item_additional_rts_by_title, 'summa_after_discount', summa_after_discount, tr_str)

    # additional_rts - Суммы по дополнительным расходам РТС
    for additional_costs_rts_item in e.additional_costs_rts:
        pb_item_additional_rts = result_estimate.additional_rts.add()
        for f in MAPPED_ADDITIONAL_RTS_NOCONV:
            set_arg(pb_item_additional_rts, f, getattr(additional_costs_rts_item, f))
        for f, tr in MAPPED_ADDITIONAL_RTS_CONV:
            set_arg_conv(pb_item_additional_rts, f, getattr(additional_costs_rts_item, f), tr)
        tmp_obj = ResolveAdditionalCostsRtsCopyFieldsOptimNewItem()
        resolve_additional_costs_rts_copy_fields(
            tmp_obj, additional_costs_rts_item, additional_costs_rts_item.price, additional_costs_rts_item.count
        )
        for f in MAPPED_ADDITIONAL_RTS_CONV_OBJ:
            set_arg_conv(pb_item_additional_rts, f, getattr(tmp_obj, f), tr_str)

        set_agency_comission(pb_item_additional_rts.agency_commission, additional_costs_rts_item.agency_commission)

    # additional_nonrts - Стоимость дополнительных услуг НОН РТС
    for additional_costs_nonrts_item in e.additional_costs_nonrts:
        additional_costs_nonrts_item_pb_item = result_estimate.additional_nonrts.add()
        map_fields(
            additional_costs_nonrts_item,
            additional_costs_nonrts_item_pb_item,
            MAPPED_FIELDS_RESOLVE_ESTIMATE_NO_SIDES_NONRTS_STR,
            MAPPED_FIELDS_RESOLVE_ESTIMATE_NO_SIDES_NONRTS_DECIMAL,
        )
        set_arg_conv(
            additional_costs_nonrts_item_pb_item,
            'city__id',
            getattr(additional_costs_nonrts_item, 'city_id'),
            city_id_to_relay_id,
        )
        set_arg(
            additional_costs_nonrts_item_pb_item, 'city__title', getattr(additional_costs_nonrts_item, 'city_title')
        )
        if resolve_estimate_no_sides_nonrts_ak_zero_predicate(additional_costs_nonrts_item.agency_commission_value):
            additional_costs_nonrts_item_pb_item.agency_commission_calculated = '0'

        set_agency_comission(
            additional_costs_nonrts_item_pb_item.agency_commission, additional_costs_nonrts_item.agency_commission
        )
        map_fields(
            additional_costs_nonrts_item,
            additional_costs_nonrts_item_pb_item.nonrts_part,
            MAPPED_ESTIMATE_NON_RTS_NOCONV,
            MAPPED_ESTIMATE_NON_RTS_DECIMAL,
            MAPPED_ESTIMATE_NON_RTS_CONV,
        )

    # reservations_nonrts - Стоимость бронирования сторон конструкций НОН РТС
    for reservations_non_rts_item in e.reservations_non_rts:
        reservations_non_rts_pb_item = result_estimate.reservations_nonrts.add()
        populate_reservation(reservations_non_rts_pb_item.reservation, reservations_non_rts_item)
        set_arg_conv_getattr(reservations_non_rts_pb_item, 'agency_commission_value', reservations_non_rts_item, tr_str)
        map_fields(
            reservations_non_rts_item.estimate_non_rts,
            reservations_non_rts_pb_item.nonrts_part,
            MAPPED_ESTIMATE_NON_RTS_NOCONV,
            MAPPED_ESTIMATE_NON_RTS_DECIMAL_RESERVATION,
            MAPPED_ESTIMATE_NON_RTS_CONV,
        )
        map_fields(
            reservations_non_rts_item,
            reservations_non_rts_pb_item.nonrts_part,
            None,
            MAPPED_FIELDS_RESOLVE_ADDITIONAL_NONRTS_RESERVATIONS_SALE_PAY_MARGIN,
        )

    # reservations -  Стоимость бронирования сторон конструкций РТС
    for reservations_rts_item in e.reservations_rts:
        reservations_rts_pb_item = result_estimate.reservations.add()
        populate_reservation(reservations_rts_pb_item.reservation, reservations_rts_item)

        map_fields(
            reservations_rts_item,
            reservations_rts_pb_item,
            None,
            MAPPED_FIELDS_CALCULATED_RESERVATION,
        )

    return ContentFieldConnection(
        content=base64_bytes(result_estimate.SerializeToString()),
        pageInfo=PageInfoApi(total_count=1, offset=0, limit=100),
    )
