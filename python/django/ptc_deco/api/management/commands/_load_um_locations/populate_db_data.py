from dataclasses import dataclass
from typing import Dict, Optional, Tuple

from ptc_deco.api import models as m
from .populate_location_str_row import LocationStrRow
from .._load_um_constructions_rts.merge_addresses import T_DB_ADDRESSES, T_DB_ADDRESS
from ..utils import get_postcode


@dataclass
class LocationDbData:
    r: LocationStrRow
    city: m.City
    district: m.District
    postcode: m.Postcode
    legal_address: m.Addresses
    marketing_address: m.Addresses
    purpose_location: m.PurposeLocation
    registration_status_location: m.RegistrationStatusLocation
    family_construction: m.FamilyConstruction


def get_or_create_or_none(get_or_create, model, value, first_arg_key, **kwargs):
    return get_or_create(model, **{first_arg_key: value}, **kwargs) if value else None


def get_address_object(
    get_or_create,
    address: str,
    db_addresses: Dict[str, T_DB_ADDRESSES],
    target_city_title: str,
    target_district_title: str,
    district: Optional[m.District],
    country: m.Country,
) -> T_DB_ADDRESS:
    addr_obj: T_DB_ADDRESS
    for addr_obj in db_addresses.get(address, []):
        if (
            target_city_title is not None
            and (addr_obj.city_title or '').lower() == (target_city_title or '').lower()
            and target_district_title is not None
            and (addr_obj.district_title or '').lower() == target_district_title.lower()
            and addr_obj.country_id == country.id
        ):
            return addr_obj

    postcode = get_or_create(m.Postcode, title='', district_id=district.id) if district else None
    addr_obj = get_or_create_or_none(
        get_or_create, m.Addresses, address, 'address', postcode_id=(postcode.id if postcode is not None else None)
    )

    return addr_obj


def populate_db_data(
    row: LocationStrRow, get_or_create, country, db_addresses: Dict[str, T_DB_ADDRESSES]
) -> LocationDbData:

    city = get_or_create(m.City, title=row.city, country_id=country.id) if row.city and country else None
    district = get_or_create(m.District, title=row.district, city_id=city.id) if city else None

    city_title = city.title if city else None
    district_title = district.title if district else None

    legal_addr_obj = get_address_object(
        get_or_create, row.address_legal, db_addresses, city_title, district_title, district, country
    )
    market_addr_obj = get_address_object(
        get_or_create, row.address_market, db_addresses, city_title, district_title, district, country
    )

    return LocationDbData(
        r=row,
        city=city,
        district=district,
        postcode=(
            market_addr_obj.postcode
            if market_addr_obj is not None
            else (legal_addr_obj.postcode if legal_addr_obj is not None else None)
        ),
        legal_address=legal_addr_obj,
        marketing_address=market_addr_obj,
        purpose_location=get_or_create_or_none(get_or_create, m.PurposeLocation, row.purpose_location, 'title'),
        registration_status_location=get_or_create_or_none(
            get_or_create, m.RegistrationStatusLocation, row.legal_status, 'title'
        ),
        family_construction=get_or_create_or_none(
            get_or_create, m.FamilyConstruction, row.family_construction, 'title'
        ),
    )
