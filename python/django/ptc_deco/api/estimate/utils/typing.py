from typing import Optional, Union

from django.db.models import F, Sum, DecimalField, Case, When, Q
from ....api import models as m

DecField = DecimalField(max_digits=20, decimal_places=4)

class AnnotationAdditionalCostsRts:
    summa_before_discount: DecField
    discount_value: DecField
    agency_commission_value: DecField
    city_title: Optional[type(m.City.title)]


T_ADDITIONAL_COSTS_RTS = Union[
    m.AdditionalCosts,
    AnnotationAdditionalCostsRts
]
