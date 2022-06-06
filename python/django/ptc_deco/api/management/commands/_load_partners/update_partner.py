from typing import Dict

from .populate_partner_row_from_db_values import PartnerDbValues
from ptc_deco.api import models as m
from ..utils import set_row_db_id, set_row_str


def update_partner(
    handled_partners: Dict[int, bool],
    partner: m.Partner,
    rv: PartnerDbValues,
):
    handled_partners[partner.id] = True
    fields_to_update = set()
    set_row_str(partner, fields_to_update, rv.r, 'bin_number')
    set_row_str(partner, fields_to_update, rv.r, 'title')
    set_row_db_id(partner, fields_to_update, rv, 'legal_address_postcode')
    set_row_db_id(partner, fields_to_update, rv, 'actual_address_postcode')
    set_row_db_id(partner, fields_to_update, rv, 'legal_address')
    set_row_db_id(partner, fields_to_update, rv, 'actual_address')
    set_row_str(partner, fields_to_update, rv.r, 'bank_recipient')
    set_row_str(partner, fields_to_update, rv.r, 'checking_account')
    set_row_str(partner, fields_to_update, rv.r, 'kbe')

    if fields_to_update:
        partner.save(update_fields=fields_to_update)
        return True

    return False
