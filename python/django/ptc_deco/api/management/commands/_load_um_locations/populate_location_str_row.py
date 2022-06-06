from dataclasses import dataclass
from datetime import datetime
from decimal import Decimal
from typing import Optional

from django.utils.timezone import make_aware


def make_aware_get(_sheet_value, _key) -> datetime:
    _value = _sheet_value.get(_key)
    return make_aware(_value) if _value else None


@dataclass
class LocationStrRow:
    row_idx: int
    city: str
    district: str
    address_market: str
    address_legal: str
    legal_status: str
    purpose_location: str
    cadastral_number: str
    family_construction: str
    comment: str
    resolution_number: str
    resolution_number_date: datetime
    area_act: str
    area_act_date: datetime
    rent_contract_number: str
    rent_contract_start: datetime
    rent_contract_end: datetime
    area: Decimal


def capitalize_or_none(val: Optional[str]):
    if val:
        return val.capitalize()
    return val


def str_to_decimal_or_none(val: str) -> Optional[Decimal]:
    return Decimal(val.replace(',', '.') if isinstance(val, str) else val) if val is not None else None


def populate_str_row(sheet_value, row_idx) -> LocationStrRow:
    sheet_value.set_row_idx(row_idx)

    return LocationStrRow(
        row_idx=row_idx,
        city=sheet_value.get_city('Город'),
        district=sheet_value.get_city('Район'),
        address_market=sheet_value.get('Маркетинговый адрес'),
        address_legal=sheet_value.get('Юридический адрес'),
        legal_status=capitalize_or_none(sheet_value.get('Статус оформления договора')),
        purpose_location=capitalize_or_none(sheet_value.get('Целевое назначение')),
        cadastral_number=sheet_value.get('Кадастровый номер'),
        family_construction=sheet_value.get('Семейство конструкции'),
        comment=sheet_value.get('Комментарий'),
        resolution_number=sheet_value.get('Номер постановления от Акимата'),
        resolution_number_date=make_aware_get(sheet_value, 'Дата постановления от Акимата'),
        area_act=sheet_value.get('Номер гос акта'),
        area_act_date=make_aware_get(sheet_value, 'Дата гос акта'),
        rent_contract_number=sheet_value.get('Номер договора'),
        rent_contract_start=make_aware_get(sheet_value, 'Дата начала договора'),
        rent_contract_end=make_aware_get(sheet_value, 'Дата окончания договора'),
        area=str_to_decimal_or_none(sheet_value.get('Площадь (га)')),
    )

