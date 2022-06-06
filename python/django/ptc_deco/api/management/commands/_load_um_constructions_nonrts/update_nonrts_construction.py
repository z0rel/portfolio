from typing import Dict, List

from ptc_deco.api import models as m
from .args_to_create_nonrts_construction import (
    AggregatedConstructionRTSDataToCreateOrUpdate,
    aggregate_construction_nonrts_data_to_create_or_update,
)
from .populate_nonrts_row_db_values import ConstructionNonRtsDbValues

from ..utils import set_row_db_id, set_row_str
from .._load_um_constructions_rts.check_and_update_construction_side_format import (
    check_and_update_construction_side_format,
)


def update_nonrts_construction(
    handled_construction: Dict[int, bool],
    construction: m.Construction,
    rv_list: List[ConstructionNonRtsDbValues]
):
    handled_construction[construction.id] = True
    fields_to_update = set()

    agg: AggregatedConstructionRTSDataToCreateOrUpdate = aggregate_construction_nonrts_data_to_create_or_update(rv_list)

    set_row_str(construction, fields_to_update, agg, 'postcode_id')

    # if set_row_db_id(construction, fields_to_update, rv, 'format'):
    #     for construction_side in construction.owned_sides.all():
    #         check_and_update_construction_side_format(
    #             rv.r.row_idx, construction, construction_side, 'on update construciton format'
    #         )

    if fields_to_update:
        construction.save(update_fields=fields_to_update)
        return True
    return False
