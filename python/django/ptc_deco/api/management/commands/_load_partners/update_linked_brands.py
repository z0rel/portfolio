from typing import List

from ptc_deco.api import models as m

from .populate_partner_row_from_db_values import PartnerDbValues

from ..utils import GetOrCreate, filter_dict


def get_or_create_brand(
        get_or_create: GetOrCreate, brand_title: str, working_sector_title: str, working_sector_id: int, handled_brands
):
    if not brand_title:
        return None  # пустые бренды пропускаются

    brands = m.Brand.objects.filter(**filter_dict({'title': brand_title, 'working_sector__title': working_sector_title}))
    if brands:
        for b in brands:
            handled_brands[b.id] = True
        return brands

    brand = get_or_create(m.Brand, **filter_dict({'title': brand_title, 'working_sector_id': working_sector_id}))
    handled_brands[brand.id] = True
    return [brand]


def update_linked_brands(get_or_create, partner: m.Partner, rv_list: List[PartnerDbValues], need_drop):
    excel_row_brands = set(
        [
            (
                rv.r.brand,
                rv.working_sector.title if rv.working_sector is not None else None,
                rv.working_sector.id if rv.working_sector is not None else None,
            )
            for rv in rv_list
        ]
    )

    partner_brands: List[m.Brand] = partner.brands.all()
    handled_brands = {v.id: False for v in partner_brands}
    partner_brands_by_titles = {}
    for brand_obj in partner_brands:
        try:
            partner_brands_by_titles[brand_obj.title].append(brand_obj)
        except KeyError:
            partner_brands_by_titles[brand_obj.title] = [brand_obj]

    for excel_row_brand_title, excel_row_working_sector_title, excel_row_working_sector_id in excel_row_brands:
        if excel_row_brand_title not in partner_brands_by_titles:
            _brands = get_or_create_brand(
                get_or_create,
                excel_row_brand_title,
                excel_row_working_sector_title,
                excel_row_working_sector_id,
                handled_brands
            )
            if _brands is not None:
                for _b in _brands:
                    partner.brands.add(_b)
        else:
            for _b in partner_brands_by_titles[excel_row_brand_title]:
                handled_brands[_b.id] = True

    if need_drop:
        unhandled_brands = [v_id for v_id, handled in handled_brands.items() if not handled]
        if unhandled_brands:
            brands_to_delete_from_link = m.Brand.objects.filter(id__in=unhandled_brands)
            for _b in brands_to_delete_from_link:
                partner.brands.remove(_b)
