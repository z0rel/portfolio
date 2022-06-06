from ptc_deco.api import models as m

from ..utils import set_row_db_id, set_row_str
from .populate_rts_row_from_sheet import ConstructionRtsStrRow
from .populate_rts_row_db_values import ConstructionRtsRowDbValues


def update_location(r: ConstructionRtsStrRow, db_row: ConstructionRtsRowDbValues, location: m.Location):
    fields_to_update = set()
    print('old location: ', location)
    set_row_str(location, fields_to_update, db_row, 'has_area')
    set_row_db_id(location, fields_to_update, db_row, 'legal_address')
    set_row_db_id(location, fields_to_update, db_row, 'marketing_address')
    set_row_db_id(location, fields_to_update, db_row, 'postcode')
    set_row_db_id(location, fields_to_update, db_row, 'family_construction')
    set_row_str(location, fields_to_update, r, 'cadastral_number')

    if fields_to_update:
        print('new location: ', location)
        location.save(update_fields=fields_to_update)
