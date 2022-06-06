from typing import Dict, Tuple, List

from .populate_nonrts_row_db_values import ConstructionNonRtsDbValues


def group_nonrts_constructions_by_sides(
    nonrts_rows: List[ConstructionNonRtsDbValues],
) -> Dict[Tuple, List[ConstructionNonRtsDbValues]]:

    constructions: Dict[Tuple, List[ConstructionNonRtsDbValues]] = {}

    for rv in nonrts_rows:
        k = (rv.address_market.id, rv.owner.id, rv.family_info.format.model.id)
        try:
            constructions[k].append(rv)
        except KeyError:
            constructions[k] = [rv]
    return constructions
