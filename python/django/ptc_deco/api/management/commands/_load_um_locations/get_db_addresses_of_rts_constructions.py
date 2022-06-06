from typing import Dict, List

from .._load_um_constructions_rts.populate_addresses_from_str import populate_addreses_from_str
from .._load_um_constructions_rts.populate_rts_row_from_sheet import ConstructionRtsStrRow
from .._load_um_constructions_rts.merge_addresses import T_DB_ADDRESSES
from ..utils import PATH_EXCEL_CONSTRUCTIONS, PATH_EXCEL_CONSTRUCTIONS_SHEET_RTS, get_sheet_value, GetOrCreate


def get_db_addresses_of_rts_constructions() -> Dict[str, T_DB_ADDRESSES]:
    c_wb, c_cols, c_sheet, c_sheet_value = get_sheet_value(PATH_EXCEL_CONSTRUCTIONS, PATH_EXCEL_CONSTRUCTIONS_SHEET_RTS)

    c_cached_xlsx_rows: List[ConstructionRtsStrRow]

    db_addresses: Dict[str, T_DB_ADDRESSES]

    (
        c_cached_xlsx_rows,
        c_sheet_value,
        c_db_cities,
        c_db_districts2,
        c_db_postcodes2,
        db_addresses,
    ) = populate_addreses_from_str(c_sheet, c_cols, GetOrCreate())
    return db_addresses
