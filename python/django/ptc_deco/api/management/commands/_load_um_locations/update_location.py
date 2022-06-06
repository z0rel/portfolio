from ptc_deco.api import models as m

from .populate_db_data import LocationDbData
from .populate_location_str_row import LocationStrRow
from ..utils import set_row_db_id, set_row_str


def update_location(location: m.Location, db_row: LocationDbData) -> True:
    fields_to_update = set()
    set_row_db_id(location, fields_to_update, db_row, 'legal_address')
    set_row_db_id(location, fields_to_update, db_row, 'marketing_address')
    set_row_db_id(location, fields_to_update, db_row, 'postcode')
    set_row_db_id(location, fields_to_update, db_row, 'family_construction')
    set_row_db_id(location, fields_to_update, db_row, 'purpose_location')
    set_row_db_id(location, fields_to_update, db_row, 'registration_status_location')
    set_row_str(location, fields_to_update, db_row.r, 'row_idx')
    set_row_str(location, fields_to_update, db_row.r, 'cadastral_number')
    set_row_str(location, fields_to_update, db_row.r, 'comment')
    set_row_str(location, fields_to_update, db_row.r, 'resolution_number')
    set_row_str(location, fields_to_update, db_row.r, 'resolution_number_date')
    set_row_str(location, fields_to_update, db_row.r, 'area_act')
    set_row_str(location, fields_to_update, db_row.r, 'area_act_date')
    set_row_str(location, fields_to_update, db_row.r, 'rent_contract_number')
    set_row_str(location, fields_to_update, db_row.r, 'rent_contract_start')
    set_row_str(location, fields_to_update, db_row.r, 'rent_contract_end')
    set_row_str(location, fields_to_update, db_row.r, 'area')
    if fields_to_update:
        location.save(update_fields=sorted(fields_to_update))
        return True
    return False
