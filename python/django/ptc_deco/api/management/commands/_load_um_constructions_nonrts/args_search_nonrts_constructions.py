from typing import Tuple

from ..utils import filter_dict


def get_args_to_search_nonrts_construction(rv_key: Tuple):
    address_id, owner_id, model_id = rv_key

    return filter_dict(
        {
            'model_id': model_id,
            'is_nonrts': True,
            'marketing_address_id': address_id,
            'nonrts_owner_id': owner_id,
        }
    )
