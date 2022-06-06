from typing import List, Dict

from .populate_partner_row_from_db_values import PartnerDbValues

from ptc_deco.api import models as m
from .update_linked_brands import update_linked_brands
from .update_linked_partners import update_linked_partners


def update_partner_linked_entities(
    get_or_create,
    all_partners_by_title: Dict[str, List[m.Partner]],
    partner: m.Partner,
    rv_list: List[PartnerDbValues],
    need_drop,
):
    update_linked_brands(get_or_create, partner, rv_list, need_drop)
    update_linked_partners(all_partners_by_title, partner, rv_list, need_drop)

