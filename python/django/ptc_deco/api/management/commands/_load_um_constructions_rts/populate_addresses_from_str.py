from functools import reduce
from typing import List, Tuple, Dict

from .merge_addresses import merge_addresses, T_DB_ADDRESSES
from .merge_city_district_postcode import (
    TD_CITIES,
    TD_DISTRICTS,
    TD_DISTRICTS2,
    TD_POSTCODES,
    TD_POSTCODES2,
    check_duplicates,
    merge_city_district_postcode,
)
from .populate_rts_row_from_sheet import ConstructionRtsStrRow, populate_rts_row_from_sheet
from ..utils import get_default_country, SheetValue


def populate_addreses_from_str(
    sheet, cols, get_or_create
) -> Tuple[List[ConstructionRtsStrRow], SheetValue, TD_CITIES, TD_DISTRICTS2, TD_POSTCODES2, Dict[str, T_DB_ADDRESSES]]:
    cached_xlsx_rows: List[ConstructionRtsStrRow] = []
    r: ConstructionRtsStrRow

    country = get_default_country(get_or_create)
    sheet_value = SheetValue(sheet, cols)

    try:
        for row_idx in range(2, sheet.max_row + 1):
            # print_row(row_idx, sheet)
            sheet_value.set_row_idx(row_idx)
            r: ConstructionRtsStrRow = populate_rts_row_from_sheet(row_idx, sheet_value)

            not_all_null = reduce(
                lambda acc, next: True if acc is not None else next,
                [getattr(r, x) for x in r.__dataclass_fields__.keys()],
            )
            if not_all_null:
                cached_xlsx_rows.append(r)

        db_cities: TD_CITIES
        db_districts: TD_DISTRICTS
        db_districts2: TD_DISTRICTS2
        db_postcodes: TD_POSTCODES
        db_postcodes2: TD_POSTCODES2

        db_cities, db_districts, db_districts2, db_postcodes, db_postcodes2 = merge_city_district_postcode(
            cached_xlsx_rows, country
        )
        db_addresses = merge_addresses(cached_xlsx_rows, db_postcodes, country, get_or_create)
        check_duplicates('After districts reduce', db_addresses, False)

        return cached_xlsx_rows, sheet_value, db_cities, db_districts2, db_postcodes2, db_addresses

    except Exception as e:
        print(f'ERROR: {e}')
        print(sheet_value.RESULT)
        raise
