from decimal import Decimal

from .copy_utils import (
    _copy_from_slots,
    _copy_model_attributes_from_dict,
    _copy_model_attributes_mapped,
    _copy_model_attributes,
)
from .......api import models as m
from ......estimate import EstimateCalc


def resolve_reservations(node_cls):
    """Получить API-таблицу бронирований"""

    mask_fields = {'appendix', 'reservation_ptr', 'estimate_non_rts'}

    def f(self, info):
        return [_copy_from_slots(node_cls(), item, mask_fields) for item in self.estimate_obj.reservations_rts]

    return f


def resolve_additional_static_itogs(node_cls):
    """ Получить API-таблицу статических итогов """

    def f(self, info):
        estimate_obj = self.estimate_obj
        result = []
        for name, value in estimate_obj.itog.static_additional_details.items():
            row = node_cls()
            row.name = name
            row.price = value
            result.append(row)
        return result

    return f


MAPPED_FIELDS_RESOLVE_ADDITIONAL_RTS_ITOGS_STR = {
    'name': 'title',
}
MAPPED_FIELDS_RESOLVE_ADDITIONAL_RTS_ITOGS_DECIMAL = {
    'summa_before_discount': 'summa_before_discount',
    'discount_value': 'discount_value',
    'agency_commission_value': 'agency_commission_value',
}
MAPPED_FIELDS_RESOLVE_ADDITIONAL_RTS_ITOGS = {
    **MAPPED_FIELDS_RESOLVE_ADDITIONAL_RTS_ITOGS_STR,
    **MAPPED_FIELDS_RESOLVE_ADDITIONAL_RTS_ITOGS_DECIMAL,
}


def get_resolve_additional_rts_itogs_discount(discount_value, summa_before_discount):
    return (
        Decimal(100) * discount_value / summa_before_discount,
        summa_before_discount - discount_value,
    )


def resolve_additional_rts_itogs(node_cls):
    """Получить API-таблицу итогов доп. расходов"""

    def f(self, info):
        estimate_obj: EstimateCalc = self.estimate_obj
        result = []
        for item in estimate_obj.additional_costs_by_title:
            row = _copy_model_attributes_from_dict(node_cls(), item, MAPPED_FIELDS_RESOLVE_ADDITIONAL_RTS_ITOGS)
            if row.summa_before_discount and row.discount_value:
                row.discount_percent, row.summa_after_discount = get_resolve_additional_rts_itogs_discount(
                    row.discount_value,
                    row.summa_before_discount
                )
            else:
                row.discount_percent = None
                row.summa_after_discount = None

            result.append(row)
        return result

    return f


MAPPED_FIELDS_RESOLVE_ESTIMATE_NO_SIDES_NONRTS_STR = {
    'name': 'title',
}
MAPPED_FIELDS_RESOLVE_ESTIMATE_NO_SIDES_NONRTS_NONRTS_SECTION_PART_DECIMAL = {
    'sale': 'nonrts_sale',
    'pay': 'nonrts_pay',
    'margin': 'nonrts_margin',
}
MAPPED_FIELDS_RESOLVE_ESTIMATE_NO_SIDES_NONRTS_DECIMAL = {
    'agency_commission_calculated': 'agency_commission_value',
}
MAPPED_FIELDS_RESOLVE_ESTIMATE_NO_SIDES_NONRTS = {
    **MAPPED_FIELDS_RESOLVE_ESTIMATE_NO_SIDES_NONRTS_STR,
    **MAPPED_FIELDS_RESOLVE_ESTIMATE_NO_SIDES_NONRTS_DECIMAL,
    **MAPPED_FIELDS_RESOLVE_ESTIMATE_NO_SIDES_NONRTS_NONRTS_SECTION_PART_DECIMAL,
    **{
        f.name: f.name
        for f in m.EstimateNonRts._meta.get_fields()
        if f.name
           not in {
               'project',
               'appendix',
               'construction_side',
               'reservation',
               'estimatereservationsnonrts',
               'estimatenosidesnonrts',
               'r_reservation',
               'reservation',
           }
    },
}


def resolve_estimate_no_sides_nonrts_ak_zero_predicate(agency_commission_calculated):
    return agency_commission_calculated is not None and agency_commission_calculated < 0


def resolve_estimate_no_sides_nonrts(node_cls):
    """Получить API-таблицу дополнительных расходов НОН РТС, не являющихся сторонами """

    def f(self, info):
        result = []
        for item in self.estimate_obj.additional_costs_nonrts:
            row = _copy_model_attributes_mapped(node_cls(), item, MAPPED_FIELDS_RESOLVE_ESTIMATE_NO_SIDES_NONRTS)
            row.agency_commission = item.agency_commission
            if resolve_estimate_no_sides_nonrts_ak_zero_predicate(row.agency_commission_calculated):
                row.agency_commission_calculated = Decimal(0)
            result.append(row)
        return result

    return f


MAPPED_FIELDS_ADDITIONAL_NONRTS_NOSIDES_DECIMAL = {
    'sale': 'nonrts_sale',
    'pay': 'nonrts_pay',
    'margin': 'nonrts_margin',
    'agency_commission_value': 'agency_commission',
}

MAPPED_FIELDS_ADDITIONAL_NONRTS_NOSIDES_STR = {
    'name': 'title',
}

MAPPED_FIELDS_ADDITIONAL_NONRTS_NOSIDES = {
    **MAPPED_FIELDS_ADDITIONAL_NONRTS_NOSIDES_STR,
    **MAPPED_FIELDS_ADDITIONAL_NONRTS_NOSIDES_DECIMAL,
}


def resolve_itogs_additional_nonrts_nosides(node_cls):
    """ Получить API-таблицу итогов по дополнительным расходам НОНРТС, не являющимся сторонами """

    def f(self, info):
        return [
            _copy_model_attributes_from_dict(node_cls(), item, MAPPED_FIELDS_ADDITIONAL_NONRTS_NOSIDES)
            for item in self.estimate_obj.additional_costs_nonrts_by_title
        ]

    return f


MAPPED_FIELDS_RESOLVE_ADDITIONAL_NONRTS_RESERVATIONS_SALE_PAY_MARGIN = {
    'sale': 'sale_nonrts',
    'pay': 'pay_nonrts',
    'margin': 'margin_nonrts',
}


def resolve_additional_nonrts_reservations(node_cls):
    """ Получить API-список бронирований НОНРТС """

    mapped_fields = {
        'format_title': 'format_title',
        'format_id': 'format_id',
        'city_title': 'city_title',
        'city_id': 'city_id',
        'address_title': 'address_title',
        'address_id': 'address_id',
        'agency_commission_value': 'agency_comission_value',
        **MAPPED_FIELDS_RESOLVE_ADDITIONAL_NONRTS_RESERVATIONS_SALE_PAY_MARGIN,
        **{
            f.name: f.name
            for f in m.Reservation._meta.get_fields()
            if f.name not in {'reservationcalculated', 'estimatereservationsnonrts', 'appendix'}
        },
    }

    def f(self, info):
        result = []
        item: m.Reservation
        for item in self.estimate_obj.reservations_non_rts:
            row = _copy_model_attributes_mapped(node_cls(), item, mapped_fields)
            row.city = item.construction_side.construction.location.postcode.district.city
            row.format = item.construction_side.construction.format
            row.address = item.construction_side.construction.location.marketing_address
            row.nonrts_part = item.estimate_non_rts
            result.append(row)
        return result

    return f


class ResolveAdditionalCostsRtsCopyFieldsOptimNewItem:
    __slots__ = (
        'summa_before_discount',
        'discount_value',
        'agency_commission_value',
        'price_after_discount',
        'discount_percent',
        'agency_commission_percent',
        'value_without_agency_commission',
        'summa_after_discount',
    )

    def log(self):
        print(", ".join([f'{k}: {getattr(self, k)}' for k in self.__slots__]))


def resolve_additional_costs_rts_copy_fields(new_item, old_item, price, count):
    new_item.summa_before_discount = old_item.summa_before_discount
    new_item.discount_value = old_item.discount_value
    new_item.agency_commission_value = old_item.agency_commission_value

    if price is not None:
        if new_item.discount_value is not None:
            new_item.price_after_discount = price - new_item.discount_value
        else:
            new_item.price_after_discount = price

    if new_item.summa_before_discount is not None:
        if new_item.discount_value is not None:
            if count is not None:
                new_item.summa_after_discount = (
                    new_item.summa_before_discount - new_item.discount_value * count
                )
                if price:
                    new_item.discount_percent = (new_item.discount_value / price) * Decimal(100)
                else:
                    new_item.discount_percent = Decimal(0)
            else:
                new_item.discount_percent = Decimal(0)
                new_item.summa_after_discount = Decimal(0)
        else:
            new_item.summa_after_discount = new_item.summa_before_discount
            new_item.discount_percent = Decimal(0)
    else:
        new_item.summa_before_discount = Decimal(0)
        new_item.discount_percent = Decimal(0)
        new_item.summa_after_discount = Decimal(0)

    if new_item.agency_commission_value is not None:
        if new_item.summa_before_discount:
            new_item.agency_commission_percent = (
                new_item.agency_commission_value / new_item.summa_before_discount
            ) * Decimal(100)
            new_item.value_without_agency_commission = new_item.summa_after_discount - new_item.agency_commission_value
    else:
        new_item.value_without_agency_commission = new_item.summa_after_discount


def resolve_additional_costs_rts(node_cls):
    """ Получить API-таблицу дополнительных расходов РТС """

    mask_fields = {}

    def f(self, info):
        estimate_obj: EstimateCalc = self.estimate_obj
        ap = []
        for item in self.estimate_obj.additional_costs_rts:
            new_item = _copy_model_attributes(node_cls(), item, mask_fields)
            resolve_additional_costs_rts_copy_fields(new_item, item, new_item.price, new_item.count)
            ap.append(new_item)
        return ap

    return f


def resolve_address_programm(node_cls):
    """ Получить API-таблицу адресной программы """

    mask_fields = {}

    def f(self, info):
        estimate_obj: EstimateCalc = self.estimate_obj
        estimate_obj.calculate_address_program()
        ap = [_copy_from_slots(node_cls(), item, mask_fields) for item in self.estimate_obj.address_programm]
        return ap

    return f
