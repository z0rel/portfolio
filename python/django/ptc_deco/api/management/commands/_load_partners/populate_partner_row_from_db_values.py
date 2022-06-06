from dataclasses import dataclass
from typing import Optional

from ptc_deco.api import models as m

from .populate_partner_row_from_sheet import PartnerStrRow
from ..utils import GetOrCreate, POSTCODE_TO_DISTRICT_MAP, get_postcode


@dataclass
class PartnerDbValues:
    r: PartnerStrRow
    legal_address: Optional[m.Addresses]
    actual_address: Optional[m.Addresses]
    working_sector: Optional[m.WorkingSector]
    partner_type: Optional[m.PartnerType]
    client_type: Optional[m.ClientType]
    legal_address_postcode: Optional[m.Postcode]
    actual_address_postcode: Optional[m.Postcode]


def populate_partner_row_from_db_values(
    get_or_create: GetOrCreate, country: m.Country, r: PartnerStrRow
) -> PartnerDbValues:

    if r.working_sector is not None:
        s_working_sector_title, s_working_sector_description = r.working_sector.split(' / ')
        working_sector = get_or_create(
            m.WorkingSector,
            title=s_working_sector_title.strip(),
            description=s_working_sector_description.strip(),
        )
    else:
        working_sector = None

    postcode = m.Postcode.objects.filter(title=r.postcode_title) if r.postcode_title else None

    if postcode:
        postcode = postcode[0]
    elif r.postcode_title:
        v = POSTCODE_TO_DISTRICT_MAP.get(r.postcode_title, None)
        if v:
            _, _, postcode = get_postcode(get_or_create, m, country, r.city_title, v, r.postcode_title)
        else:
            print(r.postcode_title)
            need_raise = True
            return None
    else:
        if r.city_title:
            _, _, postcode = get_postcode(get_or_create, m, country, r.city_title, '', '')
        else:
            postcode = None

    return PartnerDbValues(
        r=r,
        legal_address=get_or_create(m.Addresses, address=r.address) if r.address else None,
        actual_address=get_or_create(m.Addresses, address=r.address) if r.address else None,
        legal_address_postcode=postcode,
        actual_address_postcode=postcode,
        working_sector=working_sector,
        partner_type=get_or_create(m.PartnerType, title=r.partner_type) if r.partner_type else None,
        client_type=get_or_create(m.ClientType, title=r.client_type) if r.client_type else None,
    )
