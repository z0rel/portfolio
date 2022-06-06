from dataclasses import dataclass

from ptc_deco.api import models as m

@dataclass
class ConstructionRtsStrRow:
    row_idx: int
    s_city: str
    s_district: str
    s_postcode: str
    s_address_market: str
    s_address_legal: str
    s_family: str
    s_underfamily: str
    s_model: str
    s_format: str
    s_side: str
    s_size: str
    s_adv_side: str
    s_purpose_side: str
    availablity_side: str
    s_has_area: str
    s_crew: str
    status_connection: str
    cadastral_number: str
    coordinates: str
    tech_invent_number: str
    tech_phone_construction: str
    buh_invent_number: str


def populate_rts_row_from_sheet(row_idx, sheet_value) -> ConstructionRtsStrRow:
    sheet_value.set_row_idx(row_idx)
    return ConstructionRtsStrRow(
        row_idx=row_idx,
        s_city=sheet_value.get_city('Город'),
        s_district=sheet_value.get_city('Район'),
        s_postcode=sheet_value.getstr('Код района'),
        s_address_market=sheet_value.get('Маркетинговый адрес'),
        s_address_legal=sheet_value.get('Юридический адрес'),
        s_family=sheet_value.get_capitalize_family('Семейство конструкции'),
        s_underfamily=sheet_value.get_capitalize_underfamily('Подсемейство конструкции'),
        s_model=sheet_value.get('Модель'),
        s_format=sheet_value.get_capitalize_format('Формат'),
        s_side=m.construction.tr_coded_title(sheet_value.get_capitalize_side('Сторона'))[0],
        s_size=sheet_value.get('Размер (см)'),
        s_adv_side=m.construction.tr_coded_title(sheet_value.get_capitalize_adv_side('Рекламная сторона'))[0],
        s_purpose_side=sheet_value.get('Назначение стороны'),
        availablity_side=sheet_value.get('Доступность'),
        s_has_area=sheet_value.get('Наличие земли'),
        s_crew=sheet_value.get('Экипаж'),
        status_connection=sheet_value.get('Статус по подключению'),
        cadastral_number=sheet_value.get('Кадастровый номер'),
        coordinates=sheet_value.get('Координаты Отдела продаж'),
        tech_invent_number=sheet_value.getstr('Инвентарный номер (ОТО)'),
        tech_phone_construction=sheet_value.get('Номер телефона конструкции'),
        buh_invent_number=sheet_value.getstr('Инвентарный номер (бух)'),
    )
