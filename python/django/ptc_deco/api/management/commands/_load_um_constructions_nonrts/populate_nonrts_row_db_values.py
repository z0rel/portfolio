from dataclasses import dataclass
from typing import Optional, Dict, List, Tuple, Union

from ptc_deco.api import models as m
from .get_annotated_all_addresses import T_DICT_DB_ADDRESSES, T_ANNOTATED_ADDRESSES

from .populate_nonrts_row_from_sheet import ConstructionNonRtsStrRow
from .find_families_tree import find_families_tree, FamilyInfo
from .._load_um_constructions_rts.merge_addresses import T_DB_ADDRESSES
from ..utils import get_postcode


@dataclass
class ConstructionNonRtsDbValues:
    r: ConstructionNonRtsStrRow
    family_info: FamilyInfo
    owner: Optional[m.Partner]
    city: Optional[m.City]
    district: Optional[m.District]
    postcode: Optional[m.Postcode]
    address_market: Optional[m.Addresses]
    purpose_side: Optional[m.PurposeSide]
    availability_side: bool
    adv_side: Optional[m.AdvertisingSide]
    format: Optional[m.Format]


NONRTS_CITY_CODES = {'алматы': 'AL', 'нур-султан': 'NS', 'шымкент': 'SH', 'усть-каменогорск': 'UK'}


def populate_nonrts_row_db_values(
    get_or_create,
    country,
    families_tree,
    db_addresses: T_DICT_DB_ADDRESSES,
    r: ConstructionNonRtsStrRow,
):

    key = (
        r.address_market.lower() if r.address_market is not None else None,
        r.city.lower() if r.city is not None else None,
    )
    addresses_list = db_addresses.get(key, [])
    if len(addresses_list) > 1:
        print(f'Error for construction NON RTS idx: {r.row_idx}: duplicated addresses:')
        for a in addresses_list:
            print(a)

    address_obj: T_ANNOTATED_ADDRESSES = addresses_list[0] if addresses_list else None

    if address_obj and address_obj.postcode_id is not None:
        city = address_obj.city
        district = address_obj.district
        postcode = address_obj.postcode
    else:
        # print(f'UNEXISTED POSTCODE {r.city}, {r.district}, {r.address_market}')

        city, district, postcode = get_postcode(
            get_or_create,
            m,
            country,
            r.city,
            r.district,
            s_postcode=NONRTS_CITY_CODES.get(r.city.lower(), '') + 'NONRTS',
        )
        if address_obj:
            address_obj.postcode = postcode
            address_obj.save()

    if not address_obj:
        address_obj = (
            get_or_create(m.Addresses, address=r.address_market, postcode_id=postcode.id if postcode else None)
            if r.address_market
            else None
        )

    s_side = None
    if r.s_adv_side:
        s_side = r.s_adv_side.split(' ')[0]

    founded_families: FamilyInfo = find_families_tree(
        families_tree, r.s_adv_side, s_side, r.s_format, r.s_family, get_or_create
    )

    owner = get_or_create(m.Partner, title=r.owner) if r.owner else None
    if not owner.is_nonrts_owner:
        owner.is_nonrts_owner = True
        owner.save()

    return ConstructionNonRtsDbValues(
        r=r,
        family_info=founded_families,
        city=city,
        district=district,
        postcode=postcode,
        address_market=address_obj,
        owner=owner,
        purpose_side=None,
        availability_side=True,
        adv_side=founded_families.adv_side,
        format=founded_families.format
    )
