from .populate_db_data import LocationDbData
from ..utils import filter_dict


def get_args_to_create_location(db_row: LocationDbData):
    return filter_dict(
        {
            'legal_address': db_row.legal_address,
            'marketing_address': db_row.marketing_address,
            'postcode': db_row.postcode,
            'family_construction': db_row.family_construction,
            'cadastral_number': db_row.r.cadastral_number,
            'comment': db_row.r.comment,
            'resolution_number': db_row.r.resolution_number,
            'resolution_number_date': db_row.r.resolution_number_date,
            'area_act': db_row.r.area_act,
            'area_act_date': db_row.r.area_act_date,
            'rent_contract_number': db_row.r.rent_contract_number,
            'rent_contract_start': db_row.r.rent_contract_start,
            'rent_contract_end': db_row.r.rent_contract_end,
            'area': db_row.r.area,
            'purpose_location': db_row.purpose_location,
            'registration_status_location': db_row.registration_status_location,
            'row_idx': db_row.r.row_idx
        }
    )
