from datetime import datetime, timedelta
from typing import List, Tuple, Dict, Optional, Union, Set
from decimal import Decimal, getcontext
import calendar

from itertools import chain
from collections import OrderedDict
from django.db.models import F, Sum, DecimalField, Case, When, Q
from django.db.models.query import QuerySet
from django.db.models.functions import Cast, Coalesce
from loguru import logger

from ...api.models import Reservation, PlacementPrice, AdditionalCosts, EstimateNonRts
from ...api import models as m

from .utils.agency_commission_default_item import AgencyCommissionDefaultItem
from .utils.typing import T_ADDITIONAL_COSTS_RTS

from .utils.utils import sub, sub_discount, add, add2
from .utils.static_additional_cost_item import StaticAdditionalCostItem
from .utils.period_price import split_period_by_months, SplittedPeriodItem, get_period_id
from .utils.unknown_placement_price import UnknownPlacementPrice
from .utils.exception_date import ExceptionDate
from .utils.reservation_calculated import ReservationCalculated, EstimateCalculatedItogs, AddressProgrammItogs
from .utils.discount import calc_default_discount_percent, _discount_to_calc_val, Discount, calc_discount_val

DecField = DecimalField(max_digits=20, decimal_places=4)


RESERVATION_ESTIMATE_FILTER = Q(reservation_type__ikey=m.ReservationType.ReservationTypeInteger.SETTED) | Q(
    reservation_type__ikey=m.ReservationType.ReservationTypeInteger.SALED
)


class EstimateCalc:
    additional_costs_rts: Optional[T_ADDITIONAL_COSTS_RTS]

    def __init__(self, project_obj: m.Project, appendix_obj: m.Appendix = None):
        self.project_id = project_obj.id
        self.project_obj: m.Project = project_obj
        self.appendix_obj: m.Appendix = appendix_obj
        self.estimate_errors: Set[str] = set()
        # итоговая адресная программа
        self.address_programm: List[AddressProgrammItogs] = []
        self.reservations_rts: Optional[List[ReservationCalculated]] = None
        # m.Reservation
        self.reservations_non_rts: Optional[QuerySet] = None
        # Итоги по бронированиям НОН РТС
        self.reservations_non_rts_itogs: Optional[QuerySet] = None
        # Дополнительные расходы проекта / приложения
        self.additional_costs_rts: Optional[T_ADDITIONAL_COSTS_RTS] = None
        # Дополнительные расходы, разбитые в разрезе Наименование
        self.additional_costs_by_title: Optional[QuerySet] = None

        self.additional_costs_nonrts: Optional[QuerySet] = None
        self.additional_costs_nonrts_by_title: Optional[QuerySet] = None

        # Статические дополнительные расходы
        self.static_additional_costs_dict: Optional[Dict[Tuple[int, int], StaticAdditionalCostItem]] = None
        # Кортеж город-формат, используемый для разбиения сметы
        self.filter_cities_and_formats: Optional[Tuple[int, int]] = None

        # итоги по НОН РТС - неконструкционным сторонам
        self.estimate_nonrts_itogs: Optional[dict] = None
        # итоги по доп. расходам
        self.estimate_additional_itogs: Optional[dict] = None

        client_obj = project_obj and project_obj.client
        self.ak_rent = AgencyCommissionDefaultItem('to_rent', project_obj, client_obj)
        self.ak_nalog = AgencyCommissionDefaultItem('to_nalog', project_obj, client_obj)
        self.ak_print = AgencyCommissionDefaultItem('to_print', project_obj, client_obj)
        self.ak_mount = AgencyCommissionDefaultItem('to_mount', project_obj, client_obj)
        self.ak_additional = AgencyCommissionDefaultItem('to_additional', project_obj, client_obj)
        self.ak_nonrts = AgencyCommissionDefaultItem('to_nonrts', project_obj, client_obj)

        self.discount_nalog_percent = self._get_default_discount(project_obj, 'discount_nalog_percent')
        self.discount_nalog_percent_val = _discount_to_calc_val(self.discount_nalog_percent)
        self.discount_client_percent = self._get_default_discount(project_obj, 'discount_client_percent')
        self.discount_client_percent_val = _discount_to_calc_val(self.discount_client_percent)
        self.discount_price_percent = self._get_default_discount(project_obj, 'discount_price_percent')
        self.discount_price_percent_val = self._discount_to_calc_val_none(self.discount_price_percent)

        self.itog: EstimateCalculatedItogs = EstimateCalculatedItogs(project_obj, appendix_obj)

    @staticmethod
    def _discount_to_calc_val_none(discount):
        if discount is None:
            return None
        else:
            return (Decimal(100) - Decimal(discount)) / Decimal(100)

    @staticmethod
    def _get_default_discount(project_obj: m.Project, attr):
        if project_obj is not None:
            result = getattr(project_obj, attr)
            if result is not None:
                return result
            if project_obj.client is not None:
                result = getattr(project_obj.client, attr)
                if result is not None:
                    return result
        return None

    @staticmethod
    def _get_prepared_fields():
        construction_side_annotated_fields = {
            'format_title': F('construction_side__advertising_side__side__format__title'),
            'format_id': F('construction_side__advertising_side__side__format_id'),
            'city_title': F('construction_side__construction__location__postcode__district__city__title'),
            'city_id': F('construction_side__construction__location__postcode__district__city_id'),
            'address_title': F('construction_side__construction__location__marketing_address__address'),
            'address_id': F('construction_side__construction__location__marketing_address_id'),
            'reservation_type_title': F('reservation_type__title'),
            'postcode_title': F('construction_side__construction__location__postcode__title'),
            'num_in_district': F('construction_side__construction__num_in_district'),
            'side_title': F('construction_side__advertising_side__side__title'),
            'side_code': F('construction_side__advertising_side__side__code'),
            'adv_side_title': F('construction_side__advertising_side__title'),
            'adv_side_code': F('construction_side__advertising_side__code'),
            'format_code': F('construction_side__advertising_side__side__format__code'),
            'format': F('construction_side__advertising_side__side__format'),
            'is_nonrts': F('construction_side__construction__is_nonrts'),
            'nonrts_owner': F('construction_side__construction__nonrts_owner__title'),
            'status_connection': F('construction_side__construction__status_connection'),
        }
        select_related = [
            'construction_side__construction__location__postcode__district__city',
            'construction_side__construction__location__marketing_address',
            'construction_side__advertising_side__side__format',
            'agency_commission',
        ]
        selected_fields = [
            'id',
            'date_from',
            'date_to',
            'construction_side__construction__is_nonrts',
            'construction_side__advertising_side__side__format_id',
            'construction_side__construction__location__postcode__district__city',
            'construction_side__construction__location__marketing_address__address',
            'agency_commission',
            'discount_price_percent_setted',
            'rent_by_price_after_discount_setted',
            'rent_to_client_setted',
            'discount_to_client_percent_setted',
            'rent_to_client_after_discount_setted',
            'construction_side_id',
            'reservation_type_id',
            'project_id',
            'discount_nalog_percent_setted',
            'nalog_after_discount_setted',
            'construction_side',
            'reservation_type__title',
            'rent_by_price_setted',
            'mounting_setted',
            'printing_setted',
            'additional_setted',
            'nalog_setted',
            'agency_commission__value',
            'agency_commission__percent',
            'agency_commission__to_rent',
            'agency_commission__to_nalog',
            'agency_commission__to_print',
            'agency_commission__to_mount',
            'agency_commission__to_additional',
            'agency_commission__to_nonrts',
        ]
        return construction_side_annotated_fields, select_related, selected_fields

    def calc_reservation_nonrts(self, filter_params, construction_side_annotated_fields, select_related):
        # Доп. расходы НОН РТС берутся из Стороны Конструкции -> Раздел сметы НОН РТС если в бронировании они IS NULL
        # Вычисление стоимостей бронирования для конструкций НОН РТС
        nonrts_margin_expr_from_constr_side = self._nonrts_margin('construction_side__estimate_non_rts__')
        nonrts_margin_expr_from_reservation = self._nonrts_margin('estimate_non_rts__')

        nonrts_margin_rent_expr_from_constr_side = self._nonrts_margin_rent('construction_side__estimate_non_rts__')
        nonrts_margin_rent_expr_from_reservation = self._nonrts_margin_rent('estimate_non_rts__')

        nonrts_margin_tax_expr_from_constr_side = self._nonrts_margin_tax('construction_side__estimate_non_rts__')
        nonrts_margin_tax_expr_from_reservation = self._nonrts_margin_tax('estimate_non_rts__')

        nonrts_sale_expr = self._nonrts_sale('construction_side__estimate_non_rts__')
        nonrts_sale_expr_from_reservation = self._nonrts_sale('estimate_non_rts__')

        nonrts_pay_expr = self._nonrts_pay('construction_side__estimate_non_rts__')
        nonrts_pay_expr_from_reservation = self._nonrts_pay('estimate_non_rts__')

        nonrts_pay_case = Case(
            When(estimate_non_rts__isnull=False, then=nonrts_pay_expr_from_reservation), default=nonrts_pay_expr
        )

        nonrts_margin_case = Case(
            When(estimate_non_rts__isnull=False, then=nonrts_margin_expr_from_reservation),
            default=nonrts_margin_expr_from_constr_side,
        )

        nonrts_margin_rent_case = Case(
            When(estimate_non_rts__isnull=False, then=nonrts_margin_rent_expr_from_reservation),
            default=nonrts_margin_rent_expr_from_constr_side,
        )

        nonrts_margin_tax_case = Case(
            When(estimate_non_rts__isnull=False, then=nonrts_margin_tax_expr_from_reservation),
            default=nonrts_margin_tax_expr_from_constr_side,
        )

        nonrts_sale_case = Case(
            When(estimate_non_rts__isnull=False, then=nonrts_sale_expr_from_reservation), default=nonrts_sale_expr
        )

        agency_commission_nonrts = Case(
            When(
                estimate_non_rts__isnull=False,
                then=self._ak_expr(nonrts_margin_expr_from_reservation, self.ak_nonrts, ''),
            ),
            default=self._ak_expr(
                nonrts_margin_expr_from_constr_side, self.ak_nonrts, 'construction_side__estimate_non_rts__'
            ),
        )

        nonrts_prefetch_related = ['construction_side__estimate_non_rts', 'estimate_non_rts', 'appendix']
        # nonrts_query_args = [Q(estimate_non_rts__isnull=False) | Q(construction_side__estimate_non_rts__isnull=False)]

        nonrts_query_kwargs = OrderedDict(
            [
                ('construction_side__construction__is_nonrts', True),
                *filter_params.items(),
            ]
        )

        self.reservations_non_rts = (
            Reservation.objects.filter(RESERVATION_ESTIMATE_FILTER, **nonrts_query_kwargs)
            .annotate(
                **construction_side_annotated_fields,
                pay_nonrts=nonrts_pay_case,
                sale_nonrts=nonrts_sale_case,
                margin_nonrts=nonrts_margin_case,
                margin_rent=nonrts_margin_rent_case,
                margin_tax=nonrts_margin_tax_case,
                agency_comission_value=agency_commission_nonrts,
                owner=F('construction_side__construction__nonrts_owner'),
            )
            .select_related(
                *select_related,
                *[
                    'construction_side',
                    'reservation_type',
                    'project',
                    'estimate_non_rts'
                ],
            )
            .prefetch_related(*nonrts_prefetch_related)
        )

        self.reservations_non_rts_itogs = (
            Reservation.objects.filter(RESERVATION_ESTIMATE_FILTER, **nonrts_query_kwargs)
            .select_related(*select_related)
            .only('sale_nonrts', 'margin_nonrts', 'agency_commission')
            .prefetch_related(*nonrts_prefetch_related)
            .aggregate(
                sale_nonrts=Coalesce(Sum(nonrts_sale_case), 0),
                margin_nonrts=Coalesce(Sum(nonrts_margin_case), 0),
                agency_commission=Coalesce(Sum(agency_commission_nonrts), 0),
            )
        )

        self._update_nonrts_itogs(
            self.reservations_non_rts_itogs['sale_nonrts'],
            self.reservations_non_rts_itogs['margin_nonrts'],
            self.reservations_non_rts_itogs['agency_commission'],
        )

    def calc_arenda(self, **filter_params):
        """
            Расчет стоимости аренды сторон конструкций для заданного списка бронирований в разрезе
            город-адрес-формат-период

            Разбиение для адресов пока что не сделано

        @param filter_params: Параметры фильтрации броней - либо код проекта, либо код приложения
        @return:
        """
        if not filter_params:
            filter_params = {'project_id': self.project_id}

        construction_side_annotated_fields, select_related, selected_fields = self._get_prepared_fields()
        self.calc_reservation_nonrts(filter_params, construction_side_annotated_fields, select_related)

        d0 = Decimal(0)

        placement_prices_by_city_and_format = {}

        reservations_rts = (
            Reservation.objects.filter(
                RESERVATION_ESTIMATE_FILTER,
                construction_side__construction__is_nonrts=False,
                **filter_params,
            )
            .annotate(**construction_side_annotated_fields)
            .select_related(*select_related)
            .only(*selected_fields)
        )

        cities = set()
        formats = set()
        for r in chain(reservations_rts, self.reservations_non_rts):
            tuple_id = (r.city_id, r.format_id)
            cities.add(r.city_id)
            formats.add(r.format_id)
            placement_prices_by_city_and_format[tuple_id] = None

        placement_prices = PlacementPrice.objects.filter(city_id__in=cities, format_id__in=formats).only(
            'period', 'price_for_placement', 'city_id', 'format_id'
        )

        item_pp: m.PlacementPrice
        for item_pp in placement_prices:
            tuple_id = (item_pp.city_id, item_pp.format_id)
            if tuple_id in placement_prices_by_city_and_format:
                try:
                    val = placement_prices_by_city_and_format[tuple_id]
                    if val is None:
                        placement_prices_by_city_and_format[tuple_id] = [item_pp]
                    else:
                        val.append(item_pp)
                except KeyError:
                    placement_prices_by_city_and_format[tuple_id] = [item_pp]

        for k in placement_prices_by_city_and_format:
            if placement_prices_by_city_and_format[k] is None:
                self.estimate_errors.add(str(UnknownPlacementPrice(*k)))

        self.filter_cities_and_formats = placement_prices_by_city_and_format.keys()
        self.static_additional_costs_dict = StaticAdditionalCostItem.calc_static_additional_costs_dict(
            self.filter_cities_and_formats
        )

        self.reservations_rts = []

        for src_reservation in reservations_rts:
            r: ReservationCalculated = ReservationCalculated(src_reservation)
            self.reservations_rts.append(r)

            # Получить элемент доп. расходов
            cost_additional: StaticAdditionalCostItem = self.static_additional_costs_dict.get(
                (r.city_id, r.format_id), None
            )

            r.calc_arenda(self, placement_prices_by_city_and_format, cost_additional)

            additional_others_costs = (
                cost_additional.others
                if cost_additional is not None and r.additional_setted is None
                else ([('Доп. расходы', r.additional_setted)] if r.additional_setted is not None else [])
            )
            for (name, cost) in additional_others_costs:
                try:
                    self.itog.static_additional_details[name] += cost
                except KeyError:
                    self.itog.static_additional_details[name] = cost

            r.itog_summary_without_agency_commission = sub(r.itog_summary, r.itog_agency_commission)
            self.upd_add(self.itog, 'rent_by_price', r.rent_by_price_calculated)
            self.upd_add(self.itog, 'rent_by_price_discounted', r.value_after_discount_price_selected)
            self.upd_add(self.itog, 'rent_to_client', r.value_rent_to_client_selected)
            self.upd_add(self.itog, 'rent_to_client_discounted', r.value_rent_to_client_after_discount_selected)
            self.upd_add(self.itog, 'static_printing', r.additional_static_printing)
            self.upd_add(self.itog, 'static_mounting', r.additional_static_mounting)
            self.upd_add(self.itog, 'static_additional', r.additional_static_additional)
            self.upd_add(self.itog, 'nalog_before_discount', r.additional_static_nalog)
            self.upd_add(self.itog, 'nalog_after_discount', r.additional_static_nalog_value_after_discount)
            self.upd_add(self.itog, 'agency_commission_value', r.itog_agency_commission)

    # Суммирование сметных итогов
    @staticmethod
    def upd_add(dst, dstattr, value):
        if value is not None:
            setattr(dst, dstattr, getattr(dst, dstattr) + Decimal(value))

    # Суммирование сметных итогов
    @staticmethod
    def upd_add_or_set(dst, dstattr, value):
        if value is not None:
            getted = getattr(dst, dstattr)
            if getted is not None:
                setattr(dst, dstattr, getted + Decimal(value))
            else:
                setattr(dst, dstattr, Decimal(value))

    ADDITIONAL_COSTS_AK_KEYWORDS = {
        'nalog': (m.AdditionalCosts.CAT_NALOG, 'to_nalog'),
        'rent': (m.AdditionalCosts.CAT_RENT, 'to_rent'),
        'print': (m.AdditionalCosts.CAT_PRINTING, 'to_print'),
        'mount': (m.AdditionalCosts.CAT_MOUNTING, 'to_mount'),
        'additional': (m.AdditionalCosts.CAT_ADDITIONAL, 'to_additional'),
    }

    @staticmethod
    def _calc_ak_percent_expr_from_default(ak_obj: AgencyCommissionDefaultItem, summa_expr: QuerySet, prefix=''):
        return Cast((F(prefix + 'agency_commission__percent') / Cast(100, DecField)) * summa_expr, DecField)

    @staticmethod
    def _calc_ak_expr_from_default(ak_obj: AgencyCommissionDefaultItem, summa_expr: Union[QuerySet, Cast]):
        """
            Получение выражений для вычисления значения и процента агентской комиссии по умолчанию
            если она не задана для строки выброки
        @param ak_obj:
            Объект умолчательной агентской комиссии
        @param summa_expr:
            выражение, с которого вычисляется агентская комиссия
        @param prefix:
            префикс доступа к кортежу агентской комиссии в запросе
        @return:
            кортеж - выражение для вычисления значения по умолчанию агентской комиссии
                   - выражение для вычисления процента агентской комиссии на основе данных кортежа
        """
        return (
            # value если значение агентской комиссии задано и она распространяется на выбранную услугу
            (
                ak_obj.value
                if ak_obj.value is not None
                # если процент агентской комиссии задан и она распространяется на выбранную услугу
                # -> вычисленное значение на основе процента агентской комиссии по умолчанию
                #    и выражения, на которое она начисляется
                else ((ak_obj.percent_val * summa_expr) if ak_obj.percent is not None else Decimal(0))
            )
            if ak_obj.distribute
            else Decimal(0)
        )

    @staticmethod
    def _ak_expr(summa_expr, ak_obj: AgencyCommissionDefaultItem, prefix=''):
        """
            Выражение для вычисление агентской комиссии в аггрегационных итогах
        @param summa_expr:
        @return:
        """
        default_value_expr = EstimateCalc._calc_ak_expr_from_default(ak_obj, summa_expr)
        percent_expr = EstimateCalc._calc_ak_percent_expr_from_default(ak_obj, summa_expr, prefix)

        return Case(
            When(
                # Ветка - agency_commission IS NOT NULL and and agency_commission.to_nonrts
                # Агенткая комиссия задана для кортежа выборки и распростраяется на выбранную услугу
                **{prefix + 'agency_commission__isnull': False, prefix + 'agency_commission__' + ak_obj.category: True},
                then=Case(
                    # Ветка - agency_commission.value IS NOT NULL
                    #   Значение агентской комиссии задано для кортежа выборки
                    #   ->  взять вычисленное значение агентской комисии из agency_commission.value
                    When(
                        **{prefix + 'agency_commission__value__isnull': False},
                        then=F(prefix + 'agency_commission__value'),
                    ),
                    # Ветка - agency_commission.percent IS NOT NULL
                    #   Процент агентской комиссии задан для кортежа выборки
                    #   -> взять вычисленное значение агентской комисии из выражения вычисления процента агентской
                    #      комиссии
                    When(**{prefix + 'agency_commission__percent__isnull': False}, then=percent_expr),
                    # Если не задано ни значение, ни процент агентской комисси
                    # -> взять вычисленное значение агентской комиссии из значения по умолчанию
                    default=Cast(default_value_expr, DecField),
                ),
            ),
            default=Cast(default_value_expr, DecField),
        )

    @staticmethod
    def _ak_expr_additional(cost_category: int, summa_expr, ak_obj: AgencyCommissionDefaultItem, prefix=''):
        """
            Выражение для вычисление агентской комиссии в аггрегационных итогах
        @param summa_expr:
        @return:
        """

        percent_expr = EstimateCalc._calc_ak_percent_expr_from_default(ak_obj, summa_expr, prefix)

        return When(
            # Ветка - agency_commission IS NOT NULL and and agency_commission.to_nonrts
            # Агенткая комиссия задана для кортежа выборки и распростраяется на выбранную услугу
            **{prefix + 'category': cost_category, prefix + 'agency_commission__' + ak_obj.category: True},
            then=Coalesce(F(prefix + 'agency_commission__value'), percent_expr),
        )

    def calc_dynamic_additional_costs(self, **filter_specs):
        """
            Вычисление итогов по динамическим дополнительным расходам (для проекта)
            в двух разрезах 1. Город-ДатаНачала-ДатаОкончания; 2. Наименование
        @return:
        """

        # TODO: Уточнить еще раз АК начисляется на значение доп. расхода до вычета скидки или после вычета скидки
        sum_before_discount = Cast(F('count') * F('price'), DecField)
        value_before_discount = F('price')

        discount_value = Case(
            When(cost_after_discount__isnull=False, then=value_before_discount - F('cost_after_discount')),
            When(
                discount_percent__isnull=False,
                then=(value_before_discount * F('discount_percent') / Cast(100, DecField)),
            ),
            default=(
                (value_before_discount * Cast(self.discount_client_percent, DecField) / Cast(100, DecField))
                if self.discount_nalog_percent is not None
                else (Cast(0, DecField))
            ),
        )

        ak_expression = Case(
            When(
                (
                    Q(agency_commission__isnull=True)
                    | Q(agency_commission__value__isnull=True, agency_commission__percent__isnull=True)
                    | Q(
                        agency_commission__to_nalog=False,
                        agency_commission__to_mount=False,
                        agency_commission__to_print=False,
                        agency_commission__to_additional=False,
                        agency_commission__to_rent=False,
                    )
                ),
                then=Case(
                    When(
                        category=AdditionalCosts.CAT_NALOG,
                        then=EstimateCalc._calc_ak_expr_from_default(self.ak_nalog, sum_before_discount),
                    ),
                    When(
                        category=AdditionalCosts.CAT_MOUNTING,
                        then=EstimateCalc._calc_ak_expr_from_default(self.ak_mount, sum_before_discount),
                    ),
                    When(
                        category=AdditionalCosts.CAT_PRINTING,
                        then=EstimateCalc._calc_ak_expr_from_default(self.ak_print, sum_before_discount),
                    ),
                    When(
                        category=AdditionalCosts.CAT_RENT,
                        then=EstimateCalc._calc_ak_expr_from_default(self.ak_rent, sum_before_discount),
                    ),
                    When(
                        category=AdditionalCosts.CAT_ADDITIONAL,
                        then=EstimateCalc._calc_ak_expr_from_default(self.ak_additional, sum_before_discount),
                    ),
                    default=Cast(0, DecField),
                ),
            ),
            default=Case(
                self._ak_expr_additional(AdditionalCosts.CAT_NALOG, sum_before_discount, self.ak_nalog),
                self._ak_expr_additional(AdditionalCosts.CAT_MOUNTING, sum_before_discount, self.ak_mount),
                self._ak_expr_additional(AdditionalCosts.CAT_PRINTING, sum_before_discount, self.ak_print),
                self._ak_expr_additional(AdditionalCosts.CAT_RENT, sum_before_discount, self.ak_rent),
                self._ak_expr_additional(AdditionalCosts.CAT_ADDITIONAL, sum_before_discount, self.ak_additional),
                default=Cast(0, DecField),
            ),
        )

        annotated_fields_sum = OrderedDict(
            [
                ('summa_before_discount', Coalesce(Sum(sum_before_discount), 0)),
                ('discount_value', Coalesce(Sum(discount_value), 0)),
                ('agency_commission_value', Coalesce(Sum(ak_expression), 0)),
            ]
        )

        self.additional_costs_rts = (
            AdditionalCosts.objects.filter(**filter_specs)
            .select_related('agency_commission', 'city', 'project')
            .prefetch_related('appendix')
            .annotate(
                summa_before_discount=Coalesce(sum_before_discount, 0),
                discount_value=Coalesce(discount_value, 0),
                agency_commission_value=Coalesce(ak_expression, 0),
                city_title=F('city__title'),
            )
            .order_by('start_period', 'end_period', 'title')
        )

        self.additional_costs_by_title = (
            AdditionalCosts.objects.filter(**filter_specs)
            .values('title', 'category')
            .annotate(**annotated_fields_sum)
        )

        for item in self.additional_costs_by_title:
            lower_title = item['title'].lower()
            if 'брендинг' in lower_title or 'брендирован' in lower_title:
                self.upd_add_or_set(
                    self.itog, 'branding_cost', sub_discount(item['summa_before_discount'], item['discount_value'])
                )

            if item['category'] == m.AdditionalCosts.CAT_NALOG:
                self.upd_add(self.itog, 'nalog_before_discount', item['summa_before_discount'])
                self.upd_add(
                    self.itog,
                    'nalog_after_discount',
                    sub_discount(item['summa_before_discount'], item['discount_value']),
                )
            else:
                self.upd_add(self.itog, 'additional_rts_before_discount', item['summa_before_discount'])
                self.upd_add(
                    self.itog,
                    'additional_rts_after_discount',
                    sub_discount(item['summa_before_discount'], item['discount_value']),
                )

            self.upd_add(self.itog, 'agency_commission_value', item['agency_commission_value'])

    @staticmethod
    def _nonrts_margin(prefix=''):
        """
            Формирование выражения вычисления маржи (чистой прибыли) для НОН РТС расхода
        @param prefix: Префик пути к таблице EstimateNonRts в запросе
        @return: сформированное выражение вычисления маржи НОН РТС
        """

        return Cast(
            (
                Coalesce(prefix + 'sale_rent', 0)
                - Coalesce(prefix + 'incoming_rent', 0)
                + Coalesce(prefix + 'sale_tax', 0)
                - Coalesce(prefix + 'incoming_tax', 0)
                + Coalesce(prefix + 'sale_printing', 0)
                - Coalesce(prefix + 'incoming_printing', 0)
                + Coalesce(prefix + 'sale_manufacturing', 0)
                - Coalesce(prefix + 'incoming_manufacturing', 0)
                + Coalesce(prefix + 'sale_installation', 0)
                - Coalesce(prefix + 'incoming_installation', 0)
                + Coalesce(prefix + 'sale_additional', 0)
                - Coalesce(prefix + 'incoming_additional', 0)
            )
            * Coalesce(prefix + 'count', 0),
            DecField,
        )

    @staticmethod
    def _nonrts_margin_rent(prefix=''):
        """
        Формирование выражения вычисления маржи (Аренды) для НОН РТС расхода
        @param prefix: Префик пути к таблице EstimateNonRts в запросе
        @return: сформированное выражение вычисления маржи НОН РТС
        """
        return Cast(
            (Coalesce(prefix + 'sale_rent', 0) - Coalesce(prefix + 'incoming_rent', 0)) * Coalesce(prefix + 'count', 0),
            DecField,
        )

    @staticmethod
    def _nonrts_margin_tax(prefix=''):
        """
        Формирование выражения вычисления маржи (Налога) для НОН РТС расхода
        @param prefix: Префик пути к таблице EstimateNonRts в запросе
        @return: сформированное выражение вычисления маржи НОН РТС
        """
        return Cast(
            (Coalesce(prefix + 'sale_tax', 0) - Coalesce(prefix + 'incoming_tax', 0)) * Coalesce(prefix + 'count', 0),
            DecField,
        )

    @staticmethod
    def _nonrts_pay(prefix=''):
        """
            Формирование выражения вычисления маржи (чистой прибыли) для НОН РТС расхода
        @param prefix: Префик пути к таблице EstimateNonRts в запросе
        @return: сформированное выражение вычисления маржи НОН РТС
        """

        return Cast(
            (
                Coalesce(prefix + 'incoming_rent', 0)
                + Coalesce(prefix + 'incoming_tax', 0)
                + Coalesce(prefix + 'incoming_printing', 0)
                + Coalesce(prefix + 'incoming_manufacturing', 0)
                + Coalesce(prefix + 'incoming_installation', 0)
                + Coalesce(prefix + 'incoming_additional', 0)
            )
            * Coalesce(prefix + 'count', 0),
            DecField,
        )

    @staticmethod
    def _nonrts_sale(prefix=''):
        """
            Формирование выражения вычисления маржи (чистой прибыли) для НОН РТС расхода
        @param prefix: Префик пути к таблице EstimateNonRts в запросе
        @return: сформированное выражение вычисления маржи НОН РТС
        """

        return Cast(
            (
                Coalesce(prefix + 'sale_rent', 0)
                + Coalesce(prefix + 'sale_tax', 0)
                + Coalesce(prefix + 'sale_printing', 0)
                + Coalesce(prefix + 'sale_manufacturing', 0)
                + Coalesce(prefix + 'sale_installation', 0)
                + Coalesce(prefix + 'sale_additional', 0)
            )
            * Coalesce(prefix + 'count', 0),
            DecField,
        )

    def calc_dynamic_additional_costs_nonrts(self, **filter_specs):
        """
        Вычисление итогов по динамическим дополнительным расходам (НОН РТС) в разрезах (Город,) и (Наименование,)
        """

        nonrts_pay = self._nonrts_pay()
        nonrts_sale = self._nonrts_sale()
        nonrts_margin = self._nonrts_margin()
        ak_expr_nonrts = self._ak_expr(nonrts_margin, self.ak_nonrts)

        annotated_fields_nonrts = OrderedDict(
            [
                ('nonrts_pay', Coalesce(Sum(nonrts_pay), 0)),
                ('nonrts_sale', Coalesce(Sum(nonrts_sale), 0)),
                ('nonrts_margin', Coalesce(Sum(nonrts_margin), 0)),
                ('agency_commission', Coalesce(Sum(ak_expr_nonrts), 0)),
            ]
        )

        nonrts_nosides_filter = {'construction_side__isnull': True, 'reservation__isnull': True}

        # НОНРТС доп. расходы
        self.additional_costs_nonrts = (
            EstimateNonRts.objects.filter(**filter_specs, **nonrts_nosides_filter)
            .select_related('city', 'agency_commission')
            .annotate(
                nonrts_pay=Coalesce(nonrts_pay, 0),
                nonrts_sale=Coalesce(nonrts_sale, 0),
                nonrts_margin=Coalesce(nonrts_margin, 0),
                agency_commission_value=Coalesce(ak_expr_nonrts, 0),
                city_title=F('city__title')
            )
        )

        # НОНРТС доп. расходы в разрезе Наименования
        self.additional_costs_nonrts_by_title = (
            EstimateNonRts.objects.filter(**filter_specs, **nonrts_nosides_filter)
            .values('title')
            .annotate(**annotated_fields_nonrts)
        )

        self.estimate_nonrts_itogs = EstimateNonRts.objects.filter(**filter_specs, **nonrts_nosides_filter).aggregate(
            **annotated_fields_nonrts
        )

        self._update_nonrts_itogs(
            self.estimate_nonrts_itogs['nonrts_sale'],
            self.estimate_nonrts_itogs['nonrts_margin'],
            self.estimate_nonrts_itogs['agency_commission'],
        )

    def _update_nonrts_itogs(self, sale, margin, ak):
        if sale:
            self.itog.nonrts_sale += Decimal(sale)
        if margin:
            self.itog.nonrts_margin += Decimal(margin)
        if ak:
            self.itog.agency_commission_value += Decimal(ak)

    @staticmethod
    def _calc_discount(value, value_discounted):
        return Decimal(100) * (value - value_discounted) / value if value else Decimal(0)

    @staticmethod
    def fanch_report_rent_cost_rts(r: ReservationCalculated):
        d0 = Decimal(0)
        return (
            (r.value_rent_to_client_selected or d0)
            - (r.ak_rent_selected.value or d0)
            + (r.additional_static_nalog or d0)
            - (r.ak_nalog_selected.value or d0)
            + (r.additional_static_printing or d0)
            - (r.ak_print_selected.value or d0)
            + (r.additional_static_additional or d0)
            - (r.ak_addit_selected.value or d0)
            + (r.additional_static_mounting or d0)
            - (r.ak_mount_selected.value or d0)
        )

    @staticmethod
    def fanch_report_rent_cost_rts_mouth(r: ReservationCalculated):
        budget = EstimateCalc.fanch_report_rent_cost_rts(r)

        if not r.date_from or not r.date_to:
            raise ExceptionDate(r)
        if r.date_from.month == r.date_to.month:
            return budget
        else:
            diff_days = r.date_to - r.date_from
            _, days_mouth = calendar.monthrange(r.date_from.year, r.date_from.month)
            budget = budget * Decimal((days_mouth - r.date_from.day) / diff_days.days)
            return budget

    @staticmethod
    def fanch_report_rent_cost_nonrts(r):
        return Decimal(r.margin_nonrts or 0) - Decimal(r.agency_comission_value or 0)

    @staticmethod
    def fanch_report_rent_cost_non_rts_mouth(r):
        budget = EstimateCalc.fanch_report_rent_cost_nonrts(r)

        if not r.date_from or not r.date_to:
            raise ExceptionDate(r)
        if r.date_from.month == r.date_to.month:
            return budget
        else:
            diff_days = r.date_to - r.date_from
            _, days_mouth = calendar.monthrange(r.date_from.year, r.date_from.month)
            budget = budget * Decimal((days_mouth - r.date_from.day) / diff_days.days)
            return budget

    def calculate_estimate_itogs(self):
        """
        Расчет итоговых значений сметы
        """
        self.itog.rent_by_price_discount_percent = self._calc_discount(
            self.itog.rent_by_price, self.itog.rent_by_price_discounted
        )
        self.itog.rent_to_client_discount_percent = self._calc_discount(
            self.itog.rent_to_client, self.itog.rent_to_client_discounted
        )
        self.itog.nalog_discount_percent = self._calc_discount(
            self.itog.nalog_before_discount, self.itog.nalog_after_discount
        )

        self.itog.summary_estimate_value = (
            self.itog.rent_to_client_discounted
            + self.itog.static_printing
            + self.itog.static_mounting
            + self.itog.static_additional
            + self.itog.nalog_after_discount
            + self.itog.additional_rts_after_discount
            + self.itog.nonrts_sale
        )
        self.itog.summary_estimate_value_without_agency_commission = (
            self.itog.summary_estimate_value - self.itog.agency_commission_value
        )
        self.itog.agency_commission_percent = self._calc_discount(
            self.itog.summary_estimate_value, self.itog.summary_estimate_value_without_agency_commission
        )

    def calculate_address_program(self):
        # Сводка включает поля
        # Город
        # Адрес
        # Формат
        # Период
        # Аренда
        # Печать
        # Монтаж
        # Доп. расходы
        # Скидка на клиента
        # Общая сумма
        # 1. За базу по периодам взять бронирования РТС и сагрегировать их по уникальным городам, форматам и периодам
        # 2. Сгруппировать в списки по город-формат
        # 3. В каждом списке найти пересекающиеся периоды - и слить их в один:
        #     - Отсортировать по дате начала + дата окончания
        #     - Обходить с начала до конца и если дата окончания предыдущей строки меньше
        #       даты начала для текующей строки - сливать период в один

        result_dict_src = {}
        for item in self.reservations_rts:
            city_id = item.city_id
            city_title = item.city_title
            format_title = item.format_title
            format_title_key = item.format_title.lower().strip()
            key = (item.date_from, item.date_to, city_title, format_title_key, city_id)
            # try:
            #     rows_list = result_dict_src[key]
            # except KeyError:
            #     rows_list = {}
            #     result_dict_src[key] = rows_list
            try:
                r = result_dict_src[key]
                r.update(item)
            except KeyError:
                result_dict_src[key] = AddressProgrammItogs(item, city_id, format_title)

        self.address_programm = [
            result_dict_src[x].set_percents() for x in sorted(result_dict_src)
        ]

        # for item in sorted()

        # previous = None
        # fields_list = [
        #     'rent',
        #     'discount_client_value',
        #     'printing',
        #     'mounting',
        #     'additional',
        #     'nalog',
        #     'nalog_discount_value',
        #     'itog_summary',
        # ]

        # item_row = None
        # for key in sorted(result_dict_src, key=lambda x: (x.date_from, x.date_to)):
        #     rows = result_dict_src[key]
        #     for dict_key in sorted(rows):
        #         if previous is None:
        #             previous = rows[dict_key]
        #         else:
        #             item_row = rows[dict_key]
        #             if previous.date_to < item_row.date_from:
        #                 for kv in fields_list:
        #                     add2(previous, item_row, kv)
        #             else:
        #                 self.address_programm.append(previous.set_percents())
        #                 previous = item_row

        #     if previous is not None and previous != item_row:
        #         self.address_programm.append(previous.set_percents())
        #         previous = None

    def full_calc(self, **filter_params):
        old_prec = getcontext().prec
        getcontext().prec = 20
        # Расчет стоимости аренды сторон конструкций РТС и НОН РТС с начислением статических расходов
        # на аренду конструкций РТС
        if self.appendix_obj is not None and 'appendix' not in filter_params:
            self.calc_arenda(appendix=self.appendix_obj, **filter_params)
        else:
            self.calc_arenda(**filter_params)

        if self.appendix_obj:
            self.calc_dynamic_additional_costs(appendix=self.appendix_obj)
            self.calc_dynamic_additional_costs_nonrts(appendix=self.appendix_obj)
        else:
            # Вычисление итогов по динамическим дополнительным расходам (РТС)
            # в двух разрезах (Город, Период) и (Наименование,)
            self.calc_dynamic_additional_costs(project_id=self.project_id)

            # Вычисление итогов по динамическим дополнительным расходам (НОН РТС) в разрезах(Город, ) и (Наименование, )
            self.calc_dynamic_additional_costs_nonrts(project_id=self.project_id)

        # Расчет итоговых значений сметы
        self.calculate_estimate_itogs()

        getcontext().prec = old_prec
        # 1. Просуммировать в разрезе город - период, распределить на совпадающие периоды.
        # Сумму по несовпадающим периодам распределить равномерно по всем элементам разбиения.
        # 2. Просуммировать в разрезе типа доп. расходов.
        return self

    @staticmethod
    def _get_fields_tuple_to_log(item, mask_fields=None):
        if item is None:
            return []
        res = []
        for n in item._meta.get_fields():
            if not mask_fields or (n.name not in mask_fields):
                try:
                    res.append((n.name, getattr(item, n.name)))
                except Exception as e:
                    print(n.name, str(e))
        return res

    @staticmethod
    def assign_to_dst(src, dst, mask_fields=None):
        if src is None or dst is None:
            return

        for n in src._meta.get_fields():
            if not mask_fields or (n.name not in mask_fields):
                setattr(dst, n.name, getattr(src, n.name))

    @staticmethod
    def assign_to_dst_slots(src, dst, mask_fields=None):
        if src is None or dst is None:
            return

        for n in src.__slots__:
            if not mask_fields or (n not in mask_fields):
                setattr(dst, n, getattr(src, n))

    @staticmethod
    def assign_to_dst_slots_tr(src, dst, tr, mask_fields=None):
        if src is None or dst is None:
            return

        for n in src.__slots__:
            if not mask_fields or (n not in mask_fields):
                setattr(dst, n, tr(n, getattr(src, n)))

    def test_print(self):
        logger.success(f'=== {"Appendix" if self.appendix_obj else "Project"} Estimate ===')
        logger.success('Аренда')
        mask_fields = set(['appendix', 'reservation_ptr'])

        logger.success('Бронирования РТС')
        for item in self.reservations_rts:
            logger.success(self._get_fields_tuple_to_log(item, mask_fields))

        logger.success('Бронирования НОН РТС')
        for item in self.reservations_non_rts:
            logger.success(self._get_fields_tuple_to_log(item, mask_fields))

        # logger.success('Статические дополнительные расходы')
        # for appendix_key, val in self.static_additional_costs_dict.items():
        #     logger.success(('static [city, format]->', appendix_key, val))

        logger.success('Динамические дополнительные расходы - в разрезе Город - Период')
        if self.additional_costs_rts is not None:
            for item in self.additional_costs_rts:
                logger.success(self._get_fields_tuple_to_log(item, {}))

        logger.success('Динамические дополнительные расходы - в разрезе Наименования')
        if self.additional_costs_by_title is not None:
            for item in self.additional_costs_by_title:
                logger.success(item)

        logger.success('Динамические дополнительные расходы НОН РТС - все')
        if self.additional_costs_nonrts:
            for item in self.additional_costs_nonrts:
                logger.success(item)

        logger.success('Динамические дополнительные расходы НОН РТС - в разрезе Наименования')
        if self.additional_costs_nonrts_by_title:
            for item in self.additional_costs_nonrts_by_title:
                logger.success(item)

        logger.success('Итоги по бронированиям НОН РТС')
        logger.success(self.reservations_non_rts_itogs)

        logger.success('Итоги по динамическим дополнительным расходам')
        logger.success(self.estimate_additional_itogs)

        logger.success('Итоги по динамическим дополнительным расходам НОН РТС')
        logger.success(self.estimate_nonrts_itogs)

        logger.success('Итоги сметного расчета')
        if self.itog is not None:
            logger.success(self._get_fields_tuple_to_log(self.itog, {'estimatecalculateditogsapi'}))
