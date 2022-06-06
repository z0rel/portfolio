from typing import List, Tuple, Dict, Optional
from decimal import Decimal
from ptc_deco.api.models import StaticAdditionalCosts


class StaticAdditionalCostItem:
    __slots__ = ('printing', 'mounting', 'additional', 'others', 'others_nalog', 'total_sum', 'agency_commission',
                 'discount_price', 'discount_price_calc', 'discount_nalog', 'discount_nalog_calc')

    def __init__(self):
        self.printing: Optional[Decimal] = None
        self.mounting: Optional[Decimal] = None
        self.additional: Optional[Decimal] = None
        self.others: List[Tuple[str, Decimal]] = []
        self.others_nalog: Optional[Decimal] = None
        self.total_sum: Decimal = Decimal(0)
        self.agency_commission: Decimal = Decimal(0)
        self.discount_price: Optional[Decimal] = None
        self.discount_price_calc: Decimal = Decimal(1)
        self.discount_nalog: Optional[Decimal] = None
        self.discount_nalog_calc: Decimal = Decimal(1)

    def add(self, attr, value):
        if value is not None:
            attr_val = getattr(self, attr)
            setattr(self, attr, Decimal(value) if attr_val is None else Decimal(value) + attr_val)


    @staticmethod
    def calc_discount(discount):
        if discount is None:
            return Decimal(1)
        return Decimal(1) - discount / Decimal(100)

    def copy(self):
        other: StaticAdditionalCostItem = StaticAdditionalCostItem()
        other.printing = self.printing
        other.mounting = self.mounting
        other.additional = self.additional
        other.others = self.others
        other.others_nalog = self.others_nalog
        other.total_sum = self.total_sum
        other.agency_commission = self.agency_commission
        return other


    @staticmethod
    def calc_static_additional_costs_dict(filter_tuple) -> Dict[Tuple[int, int], object]:
        """
            Вычисление словаря статических дополнительных расходов, собранных в разрезе (город, формат)
        @return:
        """
        cities = [x for (x, y) in filter_tuple]
        formats = [y for (x, y) in filter_tuple]

        static_add_costs = StaticAdditionalCosts.objects.filter(city__in=cities, format__in=formats)
        static_add_costs_dict = {}

        for v in static_add_costs:
            try:
                item: StaticAdditionalCostItem = static_add_costs_dict[(v.city_id, v.format_id)]
            except KeyError:
                item: StaticAdditionalCostItem = StaticAdditionalCostItem()
                static_add_costs_dict[(v.city_id, v.format_id)] = item

            item.total_sum += Decimal(v.price)
            if v.category == StaticAdditionalCosts.CAT_PRINTING:
                item.add('printing', v.price)
            elif v.category == StaticAdditionalCosts.CAT_MOUNTING:
                item.add('mounting', v.price)
            elif v.category == StaticAdditionalCosts.CAT_NALOG:
                item.add('others_nalog', v.price)
            elif v.category == StaticAdditionalCosts.CAT_DISCOUNT_NALOG:
                item.add('discount_nalog', v.price)
                item.discount_nalog_calc = StaticAdditionalCosts.calc_discount(item.discount_nalog)
            elif v.category == StaticAdditionalCosts.CAT_DISCOUNT_PRICE:
                item.add('discount_price', v.price)
                item.discount_price_calc = StaticAdditionalCosts.calc_discount(item.discount_price)
            else:
                item.others.append((v.name, v.price))
                item.add('additional', v.price)

        return static_add_costs_dict
