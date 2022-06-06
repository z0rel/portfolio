from datetime import datetime
from typing import Optional, Dict, Tuple
from decimal import Decimal
import ptc_deco.api.models as m
from collections import OrderedDict

from .period_price import split_period_by_months, SplittedPeriodItem, get_period_id
from .unknown_placement_price import UnknownPlacementPrice
from .discount import Discount, _discount_to_calc_val
from .utils import sub, add


def _calc_discount_percent(before, discount):
    return Decimal(Decimal(100) * discount / before) if discount is not None and before is not None else None


class AgencyCommisssion:
    __slots__ = ('value', 'percent')

    def __init__(self):
        self.value: Optional[Decimal] = None
        self.percent: Optional[Decimal] = None

    def set(self, v: Tuple[Optional[Decimal], Optional[Decimal]]):
        self.value = v[0]
        self.percent = v[1]


class ReservationCalculated:
    __slots__ = (
        'id',
        'pk',
        'date_from',
        'date_to',
        'branding',
        'discount_price_percent_setted',
        'rent_by_price_after_discount_setted',
        'rent_to_client_setted',
        'discount_to_client_percent_setted',
        'rent_to_client_after_discount_setted',
        'discount_nalog_percent_setted',
        'nalog_after_discount_setted',
        'estimate_non_rts',
        'construction_side',
        'reservation_type',
        'project',
        'agency_commission',
        'appendix',
        'address_title',
        'format_title',
        'city_title',
        'rent_by_price_calculated',
        'discount_price_percent_calculated',
        'discount_price_percent_selected',
        'value_after_discount_price_calculated',
        'value_rent_to_client_selected',
        'discount_client_percent_selected',
        'ak_rent_selected',
        'ak_nalog_selected',
        'ak_mount_selected',
        'ak_print_selected',
        'ak_addit_selected',
        'value_rent_to_client_after_discount_selected',
        'additional_static_printing',
        'additional_static_mounting',
        'additional_static_additional',
        'additional_static_nalog',
        'additional_static_nalog_discount_percent_selected',
        'additional_static_nalog_discount_calculated',
        'additional_static_nalog_value_after_discount',
        'itog_summary',
        'itog_agency_commission',
        'itog_summary_without_agency_commission',
        'city',
        'format',
        'address',
        'city_id',
        'format_id',
        'address_id',
        'value_after_discount_price_selected',
        'additional_static_nalog_discount_selected',
        'rent_by_price_setted',
        'mounting_setted',
        'printing_setted',
        'additional_setted',
        'nalog_setted',
        'reservation_type_id',
        'reservation_type_title',
        'construction_side_id',
        'postcode_title',
        'num_in_district',
        'side_title',
        'side_code',
        'adv_side_title',
        'adv_side_code',
        'format_code',
        'is_nonrts',
        'nonrts_owner',
        'status_connection'
    )

    def __init__(self, r: m.Reservation):
        self.id: int = r.id
        self.pk: int = r.id

        self.date_from: Optional[datetime] = r.date_from  # Дата начала
        self.date_to: Optional[datetime] = r.date_to  # Дата окончания
        self.branding: bool = r.branding  # брендинг

        self.reservation_type_id: Optional[int] = r.reservation_type_id
        self.reservation_type_title: Optional[str] = r.reservation_type_title
        self.construction_side_id: Optional[int] = r.construction_side_id
        self.postcode_title: Optional[str] = r.postcode_title
        self.num_in_district: Optional[int] = r.num_in_district
        self.side_title: Optional[str] = r.side_title
        self.side_code: Optional[str] = r.side_code
        self.adv_side_title: Optional[str] = r.adv_side_title
        self.adv_side_code: Optional[str] = r.adv_side_code
        self.format_code: Optional[str] = r.format_code
        self.is_nonrts: Optional[bool] = r.is_nonrts
        self.nonrts_owner: Optional[str] = r.nonrts_owner
        self.status_connection: Optional[bool] = r.status_connection

        self.rent_by_price_setted: Optional[Decimal] = r.rent_by_price_setted
        self.mounting_setted: Optional[Decimal] = r.mounting_setted
        self.printing_setted: Optional[Decimal] = r.printing_setted
        self.additional_setted: Optional[Decimal] = r.additional_setted
        self.nalog_setted: Optional[Decimal] = r.nalog_setted

        # Заданный процент скидки по прайсу
        self.discount_price_percent_setted: Optional[Decimal] = r.discount_price_percent_setted
        # Заданная стоимость после скидки по прайсу
        self.rent_by_price_after_discount_setted: Optional[Decimal] = r.rent_by_price_after_discount_setted
        # Заданная аренда на клиента
        self.rent_to_client_setted: Optional[Decimal] = r.rent_to_client_setted
        # Заданный процент скидки Аренды на клиента
        self.discount_to_client_percent_setted: Optional[Decimal] = r.discount_to_client_percent_setted
        # Заданная стоимость Аренды после всех скидок
        self.rent_to_client_after_discount_setted: Optional[Decimal] = r.rent_to_client_after_discount_setted
        # Заданный процент скидки на налог
        self.discount_nalog_percent_setted: Optional[Decimal] = r.discount_nalog_percent_setted
        # Заданная стоимость налога после скидки
        self.nalog_after_discount_setted: Optional[Decimal] = r.nalog_after_discount_setted

        # Данные о НОН РТС стоимости -> Бронирование
        self.estimate_non_rts: Optional[m.EstimateNonRts] = None
        # Бронирования -> Сторона конструкции
        self.construction_side: Optional[m.ConstructionSide] = r.construction_side
        # Бронирования -> Тип брони
        self.reservation_type: Optional[m.ReservationType] = r.reservation_type
        # Бронирования -> Проект
        self.project: Optional[m.Project] = None
        # Бронирование -> Агентская комиссия
        self.agency_commission: Optional[m.AgencyCommission] = r.agency_commission
        # Бронирование <-> Приложение к договору
        self.appendix: Optional[m.Appendix] = None

        # Город
        self.city: Optional[m.City] = r.construction_side.construction.location.postcode.district.city
        self.city_id: Optional[int] = r.city_id
        self.city_title: Optional[str] = r.city_title

        # Формат
        self.format: Optional[m.Format] = r.format
        self.format_id: Optional[int] = r.format_id
        # Название города   # Наименование формата
        self.format_title: Optional[str] = r.format_title

        # Адрес
        self.address: Optional[m.Addresses] = r.construction_side.construction.location.marketing_address
        self.address_id: Optional[int] = r.address_id
        # Адрес
        self.address_title: Optional[str] = r.address_title

        # Расчитанное значение аренды по прайсу
        self.rent_by_price_calculated: Optional[Decimal] = None
        # Расчитанное значение процента скидки по прайсу
        self.discount_price_percent_calculated: Optional[Decimal] = None
        # Выбранное значение процента скидки по прайсу
        self.discount_price_percent_selected: Optional[Decimal] = None
        # Расчитанная стоимость после скидки по прайсу
        self.value_after_discount_price_calculated: Optional[Decimal] = None

        # Выбранная стоимость аренды со скидкой по прайсу
        self.value_after_discount_price_selected: Optional[Decimal] = None

        # Выбранное значение аренды на клиента
        self.value_rent_to_client_selected: Optional[Decimal] = None
        # Выбранное значение скидки на клиента
        self.discount_client_percent_selected: Optional[Decimal] = None
        # Выбранный процент агентской комиссии на аренду, Расчитанное значение агентской комиссии за аренду
        self.ak_rent_selected: AgencyCommisssion = AgencyCommisssion()
        self.ak_nalog_selected: AgencyCommisssion = AgencyCommisssion()
        self.ak_print_selected: AgencyCommisssion = AgencyCommisssion()
        self.ak_mount_selected: AgencyCommisssion = AgencyCommisssion()
        self.ak_addit_selected: AgencyCommisssion = AgencyCommisssion()
        # Выбранная стоимость аренды на клиента со скидкой по прайсу
        self.value_rent_to_client_after_discount_selected: Optional[Decimal] = None
        # Печать (из админки)
        self.additional_static_printing: Optional[Decimal] = None
        # Монтаж (из админки)
        self.additional_static_mounting: Optional[Decimal] = None
        # Доп. расходы (из админки)
        self.additional_static_additional: Optional[Decimal] = None
        # Налоги (из админки)
        self.additional_static_nalog: Optional[Decimal] = None
        # Скидка на налоги - % расчетный
        self.additional_static_nalog_discount_percent_selected: Optional[Decimal] = None
        # Скидка на налоги - расчетная
        self.additional_static_nalog_discount_calculated: Optional[Decimal] = None
        # Значение после скидки на налоги
        self.additional_static_nalog_value_after_discount: Optional[Decimal] = None

        # Выбранная скидка на налог в доп. расходах
        self.additional_static_nalog_discount_selected: Optional[Decimal] = None

        # Итого, суммарная стоимость
        self.itog_summary: Decimal = Decimal(0)
        # В т.ч. агентская комиссия
        self.itog_agency_commission: Decimal = Decimal(0)
        # Итого за вычетом агентской комиссии
        self.itog_summary_without_agency_commission: Decimal = Decimal(0)

    def calc_arenda(self, estimate, placement_prices_by_city_and_format, cost_additional):
        placement_prices = placement_prices_by_city_and_format.get((self.city_id, self.format_id), None)
        prices_dict = (
            {v.period: v.price_for_placement for v in placement_prices} if placement_prices is not None else {}
        )

        if self.rent_by_price_setted is not None:
            self.rent_by_price_calculated = self.rent_by_price_setted
        else:
            rent_by_price_calculated = Decimal(0.0)
            # Формирование разбиения периода бронирования на помесячные периоды с остатком в послденем элементе
            reservation_periods_by_months = split_period_by_months(self.date_from, self.date_to)

            item: SplittedPeriodItem
            for item in reservation_periods_by_months:
                period_id = get_period_id(item.date_from, item.period_days, item.month_from_days)
                placement_price_for_period = prices_dict.get(period_id, None)
                if placement_price_for_period is None:
                    estimate.estimate_errors.add(str(UnknownPlacementPrice(self.city_id, self.format_id, period_id)))
                else:
                    # Сумма за период бронирования
                    rent_by_price_calculated += item.period_days * (placement_price_for_period / item.month_from_days)

            # Расчитанное значение аренды по прайсу
            self.rent_by_price_calculated = rent_by_price_calculated

        # Значение скидки по прайсу
        discount_price = Discount(
            estimate.discount_price_percent,
            estimate.discount_price_percent_val,
            cost_additional.discount_price if cost_additional else None,
            cost_additional.discount_price_calc if cost_additional else None,
            self.discount_price_percent_setted,
            self.rent_by_price_calculated,
            self.rent_by_price_after_discount_setted,
        )

        self.discount_price_percent_calculated = discount_price.discount_percent_calculated
        # Расчитанная стоимость после скидки по прайсу
        self.value_after_discount_price_calculated = discount_price.value_after_discount_calculated
        # Выбранная стоимость аренды со скидкой по прайсу
        price_percent_selected = discount_price.discount_percent_selected_val
        self.value_after_discount_price_selected = discount_price.value_after_discount_selected
        self.discount_price_percent_selected = discount_price.discount_percent_selected

        # Выбранное значение аренды на клиента
        if self.rent_to_client_setted is None:
            # Аренда на клиента изначально дублирует значения
            # из параметра фактической аренды
            self.value_rent_to_client_selected = self.rent_by_price_calculated
        else:
            self.value_rent_to_client_selected = self.rent_to_client_setted

        # Выбранное значение скидки на клиента
        if self.discount_to_client_percent_setted is None:
            # Скидка на клиента изначально дублирует значения
            # из параметра скидки на фактическую аренду.
            self.discount_client_percent_selected = self.discount_price_percent_selected
        else:
            self.discount_client_percent_selected = self.discount_to_client_percent_setted
            price_percent_selected = _discount_to_calc_val(self.discount_to_client_percent_setted)

        # Выбранная стоимость аренды со скидкой на клиента
        self.value_rent_to_client_after_discount_selected = self.value_rent_to_client_selected * price_percent_selected

        # Расчитанное значение агентской комиссии за аренду
        ak_rent = estimate.ak_rent.copy().update('to_rent', self.agency_commission)
        self.ak_rent_selected.set(ak_rent.calc(self.value_rent_to_client_selected))

        self.additional_static_printing = self.select_additional(self.printing_setted, cost_additional, 'printing')
        ak_print = estimate.ak_print.copy().update('to_print', self.agency_commission)
        self.ak_print_selected.set(ak_print.calc(self.additional_static_printing))

        self.additional_static_mounting = self.select_additional(self.mounting_setted, cost_additional, 'mounting')
        ak_mount = estimate.ak_mount.copy().update('to_mount', self.agency_commission)
        self.ak_mount_selected.set(ak_mount.calc(self.additional_static_mounting))

        self.additional_static_additional = self.select_additional(
            self.additional_setted, cost_additional, 'additional'
        )
        ak_additional = estimate.ak_additional.copy().update('to_additional', self.agency_commission)
        self.ak_addit_selected.set(ak_additional.calc(self.additional_static_additional))

        self.additional_static_nalog = self.select_additional(self.nalog_setted, cost_additional, 'others_nalog')
        ak_nalog = estimate.ak_nalog.copy().update('to_nalog', self.agency_commission)
        self.ak_nalog_selected.set(ak_nalog.calc(self.additional_static_nalog))

        discount_nalog = Discount(
            estimate.discount_nalog_percent,
            estimate.discount_nalog_percent_val,
            cost_additional.discount_nalog if cost_additional is not None else 0,
            cost_additional.discount_nalog_calc if cost_additional is not None else 0,
            self.discount_nalog_percent_setted,
            self.additional_static_nalog,
            self.nalog_after_discount_setted,
        )

        self.additional_static_nalog_discount_selected = discount_nalog.value_after_discount_calculated
        self.additional_static_nalog_value_after_discount = discount_nalog.value_after_discount_selected
        self.additional_static_nalog_discount_percent_selected = discount_nalog.discount_percent_selected

        self.upd_add_summary(self.value_rent_to_client_after_discount_selected)
        self.upd_add_summary(self.additional_static_printing)
        self.upd_add_summary(self.additional_static_mounting)
        self.upd_add_summary(self.additional_static_additional)
        self.upd_add_summary(self.additional_static_nalog_value_after_discount)
        self.upd_add_itog_agency_commission(self.ak_rent_selected.value)
        self.upd_add_itog_agency_commission(self.ak_print_selected.value)
        self.upd_add_itog_agency_commission(self.ak_mount_selected.value)
        self.upd_add_itog_agency_commission(self.ak_addit_selected.value)
        self.upd_add_itog_agency_commission(self.ak_nalog_selected.value)

    def upd_add_summary(self, value):
        if value is not None:
            self.itog_summary += Decimal(value)

    def upd_add_itog_agency_commission(self, value):
        if value is not None:
            self.itog_agency_commission += Decimal(value)

    @staticmethod
    def select_additional(setted, cost_additional, attr):
        if setted:
            return setted
        if cost_additional:
            return getattr(cost_additional, attr)
        return None


class EstimateCalculatedItogs:
    __slots__ = (
        'rent_by_price',
        'rent_by_price_discounted',
        'rent_by_price_discount_percent',
        'rent_to_client',
        'rent_to_client_discounted',
        'rent_to_client_discount_percent',
        'static_printing',
        'static_mounting',
        'static_additional',
        'nalog_before_discount',
        'nalog_after_discount',
        'nalog_discount_percent',
        'agency_commission_value',
        'agency_commission_percent',
        'nonrts_margin',
        'nonrts_sale',
        'additional_rts_before_discount',
        'additional_rts_after_discount',
        'summary_estimate_value',
        'summary_estimate_value_without_agency_commission',
        'project',
        'project_id',
        'appendix',
        'appendix_id',
        'static_additional_details',
        'branding_cost',
    )

    def __init__(self, project_obj: Optional[m.Project], appendix_obj: Optional[m.Appendix]):
        # Аренда по прайсу суммарная
        self.rent_by_price: Decimal = Decimal(0)
        # Аренда по прайсу со скидкой по прйсу суммарная
        self.rent_by_price_discounted: Decimal = Decimal(0)
        # Итоговый процент скидки по прайсу
        self.rent_by_price_discount_percent: Optional[Decimal] = None
        # Аренда на клиента суммарная
        self.rent_to_client: Decimal = Decimal(0)
        # Аренда на клиента суммарная со скидкой на клиента
        self.rent_to_client_discounted: Decimal = Decimal(0)
        # Итоговый процент скидки на клиента
        self.rent_to_client_discount_percent: Optional[Decimal] = None
        # Итоговая доп. работа - печать
        self.static_printing: Decimal = Decimal(0)
        # Итоговая доп. работа - монтаж
        self.static_mounting: Decimal = Decimal(0)
        # Итоговая доп. работа - доп. расходы
        self.static_additional: Decimal = Decimal(0)
        # Итоговый налог до скидки
        self.nalog_before_discount: Decimal = Decimal(0)
        # Итоговый налог после скидки
        self.nalog_after_discount: Decimal = Decimal(0)
        # Итоговый процент скидки по налогу
        self.nalog_discount_percent: Optional[Decimal] = None
        # Итоговое значение агентской комиссии
        self.agency_commission_value: Decimal = Decimal(0)
        # Итоговый процент агентской комиссии
        self.agency_commission_percent: Optional[Decimal] = None
        # Итоговая маржа НОН РТС
        self.nonrts_margin: Decimal = Decimal(0)
        # Итоговая стоимость продажи НОН РТС
        self.nonrts_sale: Decimal = Decimal(0)
        # Итоговая стоимость доп. расходов РТС
        self.additional_rts_before_discount: Decimal = Decimal(0)
        # Итоговая стоимость доп. расходов РТС
        self.additional_rts_after_discount: Decimal = Decimal(0)
        # Итоговое значение сметы
        self.summary_estimate_value: Optional[Decimal] = None
        # Итоговое значение сметы за вычетом агентской комиссии
        self.summary_estimate_value_without_agency_commission: Optional[Decimal] = None
        # Бронирования -> Проект
        self.project: Optional[m.Project] = project_obj
        self.project_id: Optional[int] = project_obj.id if project_obj is not None else None
        # Бронирование <-> Приложение к договору
        self.appendix: Optional[m.Appendix] = appendix_obj
        self.appendix_id: Optional[int] = appendix_obj.id if appendix_obj is not None else None
        # Итоговая доп. работа - детальный словарь
        self.static_additional_details: OrderedDict[str, Decimal] = OrderedDict()
        # Стоимость брендинга
        self.branding_cost: Optional[Decimal] = None


class AddressProgrammItogs:
    __slots__ = (
        'id',
        'city_title',
        'city_id',
        'format_title',
        'format_id',
        'date_from',
        'date_to',
        'rent',
        'discount_client_percent',
        'discount_client_value',
        'printing',
        'mounting',
        'additional',
        'nalog',
        'nalog_discount_percent',
        'nalog_discount_value',
        'itog_summary',
        'count'
    )

    def __init__(self, item: ReservationCalculated, city_id: Optional[int], format_title: Optional[str]):
        self.id: Optional[int] = item.id
        # Название города
        self.city_title: Optional[str] = item.city_title
        self.city_id: Optional[int] = city_id
        # Наименование формата
        self.format_title: Optional[str] = format_title
        self.format_id: Optional[int] = item.format_id
        # Дата начала
        self.date_from: Optional[datetime] = item.date_from
        # Дата окончания
        self.date_to: Optional[datetime] = item.date_to
        # Расчитанное значение аренды на клиента (без скидки)
        self.rent: Optional[Decimal] = item.value_rent_to_client_selected
        # Расчитанное значение процента скидки по прайсу
        self.discount_client_percent: Optional[Decimal] = None
        # Расчитанное значение стоимости скидки по прайсу
        self.discount_client_value: Optional[Decimal] = sub(
            item.value_rent_to_client_selected, item.value_rent_to_client_after_discount_selected
        )
        # Стоимость печати
        self.printing: Optional[Decimal] = item.additional_static_printing
        # Стоимость монтажа
        self.mounting: Optional[Decimal] = item.additional_static_mounting
        # Стоимость дополнительных расходов
        self.additional: Optional[Decimal] = item.additional_static_additional
        # Стоимость налогов
        self.nalog: Optional[Decimal] = item.additional_static_nalog
        # Процент скидки на налог
        self.nalog_discount_percent: Optional[Decimal] = None
        # Стоимость налога после скидки
        self.nalog_discount_value: Optional[Decimal] = sub(
            item.additional_static_nalog, item.additional_static_nalog_value_after_discount
        )
        # Итого, суммарная стоимость
        self.itog_summary: Optional[Decimal] = item.itog_summary
        # Количество
        self.count = 1

    def update(self, item: ReservationCalculated):
        self.count += 1
        self.rent = add(self.rent, item.value_rent_to_client_selected)
        self.discount_client_value = add(
            self.discount_client_value,
            sub(item.value_rent_to_client_selected, item.value_rent_to_client_after_discount_selected),
        )
        self.printing = add(self.printing, item.additional_static_printing)
        self.mounting = add(self.mounting, item.additional_static_mounting)
        self.additional = add(self.additional, item.additional_static_additional)
        self.nalog = add(self.nalog, item.additional_static_nalog)
        self.nalog_discount_value = add(
            self.nalog_discount_value,
            sub(item.additional_static_nalog, item.additional_static_nalog_value_after_discount),
        )
        self.itog_summary = add(self.itog_summary, item.itog_summary)

    def set_percents(self):
        self.discount_client_percent = _calc_discount_percent(self.rent, self.discount_client_value)
        self.nalog_discount_percent = _calc_discount_percent(self.rent, self.discount_client_value)
        return self
