from typing import Dict

from .populate_db_data import LocationDbData
from ..utils import filter_dict


def get_args_search_location(db_row: LocationDbData) -> Dict:
    return filter_dict(
        {
            'legal_address': db_row.legal_address,
            'marketing_address': db_row.marketing_address,
            'postcode': db_row.postcode,
            'family_construction': db_row.family_construction,
            'cadastral_number': db_row.r.cadastral_number,
        }
    )
