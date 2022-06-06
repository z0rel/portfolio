from typing import Dict

from ptc_deco.api import models as m

from ..utils import set_row_db_id, set_row_str
from .populate_rts_row_db_values import ConstructionRtsRowDbValues
from .check_and_update_construction_side_format import check_and_update_construction_side_format


def update_construction_side(
    row_idx,
    handled_construction_sides: Dict[int, bool],
    construction_side: m.ConstructionSide,
    construction: m.Construction,
    rv: ConstructionRtsRowDbValues,
):
    handled_construction_sides[construction_side.id] = True
    check_and_update_construction_side_format(row_idx, construction, construction_side, 'on update construction side')

    fields_to_upadte = set()
    set_row_db_id(construction_side, fields_to_upadte, rv, 'purpose_side')
    set_row_str(construction_side, fields_to_upadte, rv, 'availability_side')

    if construction.crews_has_special_for_sides and rv.crew is not None:
        set_row_db_id(construction_side, fields_to_upadte, rv, 'crew')

    if fields_to_upadte:
        construction_side.save(update_fields=fields_to_upadte)
