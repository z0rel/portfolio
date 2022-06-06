from .populate_partner_row_from_db_values import PartnerDbValues
from ..utils import filter_dict


def get_args_to_search_partners(rv: PartnerDbValues):
    return filter_dict(
        {
            'bin_number': rv.r.bin_number,
            'title': rv.r.title,
        }
    )
