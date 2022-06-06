from typing import Tuple, Dict

from .types import T_LIST_OF_RTS_DB_ROW


def group_rts_constructions_by_sides(rts_rows_with_location: T_LIST_OF_RTS_DB_ROW) -> Dict[Tuple, T_LIST_OF_RTS_DB_ROW]:
    constructions = {}
    for rv in rts_rows_with_location:
        k = (rv.r.tech_invent_number, rv.r.buh_invent_number, rv.format.model_id if rv.format else None)
        try:
            constructions[k].append(rv)
        except KeyError:
            constructions[k] = [rv]
    return constructions
