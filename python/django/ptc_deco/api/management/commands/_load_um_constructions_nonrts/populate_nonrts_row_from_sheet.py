from dataclasses import dataclass


@dataclass
class ConstructionNonRtsStrRow:
    row_idx: int
    city: str
    district: str
    address_market: str
    s_family: str
    s_format: str
    s_adv_side: str
    owner: str


def populate_nonrts_row_from_sheet(row_idx, sheet_value, cols) -> ConstructionNonRtsStrRow:
    sheet_value.set_row_idx(row_idx)
    return ConstructionNonRtsStrRow(
        row_idx=row_idx,
        city=sheet_value.get_city('Город'),
        district=sheet_value.get_city('Район'),
        address_market=sheet_value.get('Адрес'),
        s_family=sheet_value.get_capitalize_family('Семейство конструкции'),
        s_format=sheet_value.get_capitalize_format('Формат'),
        s_adv_side=sheet_value.get_capitalize_adv_side_by_idx(cols['Формат'] + 1),
        owner=sheet_value.get('Владелец РК'),
    )
