from decimal import Decimal
from typing import Tuple, Optional
from ptc_deco.api.models import Project, Partner, AgencyCommission


class AgencyCommissionDefaultItem:
    __slots__ = ('distribute', 'percent', 'percent_val', 'value', 'category')

    def __init__(self, category=None, p: Optional[Project]=None, c: Optional[Partner]=None):
        self.distribute: bool = False
        self.percent: Optional[Decimal] = None
        self.percent_val: Decimal = Decimal(0)
        self.value: Optional[Decimal] = None
        self.category: str = category

        if category is None:
            return

        self.distribute = ((p and p.agency_commission and getattr(p.agency_commission, category))
                           or (c and c.agency_commission and getattr(c.agency_commission.to_nalog, category))
                           or False)

        self.percent = self.tr_percent(((p and p.agency_commission and p.agency_commission.percent)
                                        or (c and c.agency_commission and c.agency_commission.percent)
                                        or None))
        if self.percent is not None:
            self.percent_val = self.percent / Decimal(100)

        self.value = ((p and p.agency_commission and p.agency_commission.value)
                      or (c and c.agency_commission and c.agency_commission.value)
                      or None)

        if self.value is not None:
            self.value = Decimal(self.value)

        if self.percent is None and self.value is None:
            self.distribute = False

        if not self.distribute:
            self.value = None
            self.percent = None
            self.percent_val = Decimal(0)

    def calc(self, source_value: Decimal) -> Tuple[Optional[Decimal], Optional[Decimal]]:
        """
            Вычислить стоимость агентской комиссии по заданному объекту
        @param ak_obj:
        @param summary_value:
        @return:
        """
        if source_value is None or not self.distribute:
            return None, None
        elif self.value is not None:
            return self.value, self.value * Decimal(100) / source_value
        elif self.percent is not None:
            return source_value * self.percent_val, self.percent
        else:
            return None, None

    def update(self, category, ak: AgencyCommission):
        if ak is None or category is None:
            return self

        if getattr(ak, category) and (ak.percent is not None or ak.value is not None):
            self.distribute = True
            self.value = Decimal(ak.value) if ak.value is not None else None
            self.percent = self.tr_percent(ak.percent) if ak.percent is not None else None
            self.percent_val = self.percent / Decimal(100) if self.percent is not None else Decimal(0)

        return self

    def copy(self):
        new_ak_item = AgencyCommissionDefaultItem()
        new_ak_item.percent = self.percent
        new_ak_item.percent_val = self.percent_val
        new_ak_item.value = self.value
        new_ak_item.distribute = self.distribute
        return new_ak_item


    @staticmethod
    def tr_percent(percent_db) -> Decimal:
        return Decimal(percent_db) if percent_db is not None else None
