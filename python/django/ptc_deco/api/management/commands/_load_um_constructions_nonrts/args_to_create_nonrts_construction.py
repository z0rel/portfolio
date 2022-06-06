from dataclasses import dataclass
from typing import List, Tuple, Optional

from ptc_deco.api import models as m

from .populate_nonrts_row_db_values import ConstructionNonRtsDbValues
from .._load_um_constructions_rts.args_to_create_rts_construction import get_attr_and_check
from ..utils import filter_dict


@dataclass
class AggregatedConstructionRTSDataToCreateOrUpdate:
    postcode_id: Optional[str]
    postcodes: List[int]


def aggregate_construction_nonrts_data_to_create_or_update(rv_list: List[ConstructionNonRtsDbValues]):
    postcodes: List[int] = get_attr_and_check(rv_list, 'postcode', m.Postcode)
    postcode_id: Optional[int] = postcodes[0] if postcodes else None

    return AggregatedConstructionRTSDataToCreateOrUpdate(postcode_id=postcode_id, postcodes=postcodes)


def get_args_to_create_nonrts_construction(rv_key: Tuple, rv_list: List[ConstructionNonRtsDbValues]):
    address_id, owner_id, model_id = rv_key

    agg: AggregatedConstructionRTSDataToCreateOrUpdate = aggregate_construction_nonrts_data_to_create_or_update(rv_list)

    return filter_dict(
        {
            'model_id': model_id,
            'is_nonrts': True,
            'marketing_address_id': address_id,
            'nonrts_owner_id': owner_id,
            'postcode_id': agg.postcode_id,
        }
    )
