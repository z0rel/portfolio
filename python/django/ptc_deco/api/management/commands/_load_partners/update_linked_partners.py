from typing import List, Dict

from .populate_partner_row_from_db_values import PartnerDbValues
from ptc_deco.api import models as m


def update_linked_partners(
    all_partners_by_title: Dict[str, List[m.Partner]],
    partner: m.Partner,
    rv_list: List[PartnerDbValues],
    need_drop,
):
    excel_row_partners = set([rv.r.linked_partner for rv in rv_list if rv.r.linked_partner])
    linked_partners_objs: List[m.Partner] = partner.advertisers.all()
    handled_partners = {v.id: False for v in linked_partners_objs}

    existed_linked_partners_by_titles = {}
    for partner_obj in linked_partners_objs:
        try:
            existed_linked_partners_by_titles[partner_obj.title].append(partner_obj)
        except KeyError:
            existed_linked_partners_by_titles[partner_obj.title] = [partner_obj]

    for partner_title in excel_row_partners:
        if partner_title not in existed_linked_partners_by_titles:
            _partners = all_partners_by_title.get(partner_title, None)
            if _partners is None:
                indices = ','.join([str(x) for x in sorted([rv.r.row_idx for rv in rv_list])])
                print(
                    f"""Error for partner. Linked partner is not existed {partner.title} -> { partner_title 
                    }, row_idx: {indices}"""
                )
            else:
                for _p in _partners:
                    handled_partners[_p.id] = True
                    partner.advertisers.add(_p)
        else:
            for _p in existed_linked_partners_by_titles[partner_title]:
                handled_partners[_p.id] = True

    if need_drop:
        unhandled_partners = [v_id for v_id, handled in handled_partners.items() if not handled]
        if unhandled_partners:
            partners_to_delete_from_link = m.Partner.objects.filter(id__in=unhandled_partners)
            for _p in partners_to_delete_from_link:
                partner.advertisers.remove(_p)
