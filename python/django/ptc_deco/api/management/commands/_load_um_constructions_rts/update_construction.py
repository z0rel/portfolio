from typing import Dict, List

from ptc_deco.api import models as m
from .args_to_create_rts_construction import (
    aggregate_construction_data_to_create_or_update,
    AggregatedConstructionDataToCreateOrUpdate,
)

from ..utils import set_row_db_id, set_row_str
from .check_and_update_construction_side_format import check_and_update_construction_side_format
from .populate_rts_row_from_sheet import ConstructionRtsStrRow
from .populate_rts_row_db_values import ConstructionRtsRowDbValues


def update_construction(
    handled_construction: Dict[int, bool],
    construction: m.Construction,
    r: ConstructionRtsStrRow,
    rv: ConstructionRtsRowDbValues,
    rv_list: List[ConstructionRtsRowDbValues],
):
    handled_construction[construction.id] = True

    agg: AggregatedConstructionDataToCreateOrUpdate = aggregate_construction_data_to_create_or_update(rv_list)

    fields_to_update = set()

    upd_location_id = set_row_str(construction, fields_to_update, agg, 'location_id')
    set_row_str(construction, fields_to_update, agg, 'crew_id')
    upd_tech_number = set_row_str(construction, fields_to_update, agg, 'tech_invent_number')
    upd_buh_number = set_row_str(construction, fields_to_update, agg, 'buh_invent_number')
    set_row_str(construction, fields_to_update, agg, 'tech_phone_construction')
    set_row_str(construction, fields_to_update, agg, 'coordinates')
    set_row_str(construction, fields_to_update, agg, 'status_connection')
    set_row_str(construction, fields_to_update, agg, 'is_nonrts')
    set_row_str(construction, fields_to_update, agg, 'marketing_address_id')
    set_row_str(construction, fields_to_update, agg, 'legal_address_id')
    set_row_str(construction, fields_to_update, agg, 'postcode_id')

    upd_model_id = set_row_str(construction, fields_to_update, agg, 'model_id')

    if upd_location_id or upd_tech_number or upd_buh_number or upd_model_id:
        q_construction = m.Construction.objects.filter(
            location_id=rv.location.id if rv.location is not None else None,
            tech_invent_number=r.tech_invent_number,
            buh_invent_number=r.buh_invent_number,
            model_id=rv.format.model.id if rv.format is not None else None,
        )
        if q_construction and q_construction[0].id != construction.id:
            instance = q_construction[0]
            print(
                f"""Error: inconsistance row idx of construction {construction.row_idx} buh number construction: {
                    construction.buh_invent_number}, row row_idx: {r.row_idx} row buh number: {
                    rv.r.buh_invent_number}.\n{''
                    }  Unique is changed to existed instance with id={instance.id} and row_idx={
                    instance.row_idx}"""
            )
            return False

    if upd_model_id:
        for construction_side in construction.owned_sides.all():
            check_and_update_construction_side_format(
                r.row_idx, construction, construction_side, 'on update construciton format'
            )

    if len(fields_to_update) > 0:
        try:
            construction.save(update_fields=fields_to_update)
        except Exception as e:
            print(e)
            print(
                f"""Error: inconsitance row idx of construction {construction.row_idx} buh number construction: {
                    construction.buh_invent_number}, row row_idx: {rv.r.row_idx} row buh number: {
                    rv.r.buh_invent_number} """
            )
            return False
        return True
    return False
