from typing import List, Dict, Tuple, Optional, Union

from .populate_partner_row_from_db_values import PartnerDbValues

T_GROUPS_OF_PARTNERS_KEY = Tuple[Optional[str], Optional[Union[str, int]]]
T_GROUPS_OF_PARTNERS_VALUES = List[PartnerDbValues]
T_GROUPS_OF_PARTNERS = Dict[T_GROUPS_OF_PARTNERS_KEY, T_GROUPS_OF_PARTNERS_VALUES]


def group_partners_by_key(db_rows: List[PartnerDbValues]) -> T_GROUPS_OF_PARTNERS:
    partners = {}

    for rv in db_rows:
        k = (rv.r.title, rv.r.bin_number)
        try:
            partners[k].append(rv)
        except KeyError:
            partners[k] = [rv]

    return partners
