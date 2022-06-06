from dataclasses import dataclass
from typing import Optional, Dict, List, Union

from ptc_deco.api import models as m

from ..utils import get_family_objects, GetOrCreate
from .merge_city_district_postcode import TD_CITIES, TD_DISTRICTS2, TD_POSTCODES, TD_POSTCODES2
from .populate_rts_row_from_sheet import ConstructionRtsStrRow


def get_loc_code(city, district, postcode):
    return f'{(city and city.title) or ""} {(district and district.title) or ""} {(postcode and postcode.title) or ""}'


def convert_yes_to_bool(value):
    return True if value and value.lower() == 'да' else False


@dataclass
class ConstructionRtsRowDbValues:
    city: Optional[m.City]
    district: Optional[m.District]
    postcode: Optional[m.Postcode]
    legal_address: Optional[m.Addresses]
    marketing_address: Optional[m.Addresses]
    family: Optional[m.FamilyConstruction]
    family_construction: Optional[m.FamilyConstruction]
    underfamily: Optional[m.UnderFamilyConstruction]
    model: Optional[m.ModelConstruction]
    format: Optional[m.Format]
    side: Optional[m.Side]
    adv_side: Optional[m.AdvertisingSide]
    purpose_side: Optional[m.PurposeSide]
    availability_side: bool
    has_area: bool
    status_connection: bool
    crew: Optional[m.Crew]
    location: Optional[m.Location]
    is_nonrts: bool
    r: ConstructionRtsStrRow
    loc_key: Optional[str]
    parsed_construction: Optional[m.Construction]


def check_row_is_empty(r: ConstructionRtsStrRow):
    return (
        r.s_district is None
        and r.s_city is None
        and r.s_postcode is None
        and r.cadastral_number is None
        and r.s_address_market is None
        and r.s_address_legal is None
    )


def populate_db_row(
    get_or_create: GetOrCreate,
    r: ConstructionRtsStrRow,
    db_cities: TD_CITIES,
    db_districts2: TD_DISTRICTS2,
    db_postcodes2: TD_POSTCODES2,
    addrdict: Dict[type(m.Addresses.address), List[m.Addresses]],
    start_indices: Dict[str, int],
) -> Optional[ConstructionRtsRowDbValues]:

    if check_row_is_empty(r):
        return None

    # В случае отсутствия дубликатов, списки должны содержать по 1 уникальному элементу
    city = db_cities[r.s_city][0]
    try:
        district = db_districts2[(r.s_district, city.id)][0]
    except KeyError as key:
        print(f'Unexisted district {r.s_district}, city: {city.id}, {city.title}')
        print(r)
        raise

    try:
        postcode = db_postcodes2[(r.s_postcode, district.id, city.id)][0]
    except KeyError as key:
        print(
            f'Unexisted postcode {r.s_postcode}, district: {district.id}, {district.title}, city: {city.id}, {city.title}'
        )
        raise

    loc_code = get_loc_code(city, district, postcode)
    if loc_code not in start_indices:
        start_indices[loc_code] = 1

    family, underfamily, model, format, side, adv_side = get_family_objects(
        get_or_create,
        m,
        r.s_family,
        r.s_underfamily,
        r.s_model,
        r.s_format,
        r.s_side,
        r.s_size,
        r.s_adv_side,
    )

    return ConstructionRtsRowDbValues(
        r=r,
        city=city,
        district=district,
        postcode=postcode,
        legal_address=(
            [x for x in addrdict[r.s_address_legal] if x.postcode_id == postcode.id][0] if r.s_address_legal else None
        ),
        marketing_address=(
            [x for x in addrdict[r.s_address_market] if x.postcode_id == postcode.id][0] if r.s_address_market else None
        ),
        family=family,
        family_construction=family,
        underfamily=underfamily,
        model=model,
        format=format,
        side=side,
        adv_side=adv_side,
        purpose_side=get_or_create(m.PurposeSide, title=r.s_purpose_side) if r.s_purpose_side else None,
        availability_side=convert_yes_to_bool(r.availablity_side),
        has_area=convert_yes_to_bool(r.s_has_area),
        status_connection=convert_yes_to_bool(r.status_connection),
        crew=get_or_create(m.Crew, name=r.s_crew, city_id=city.id) if r.s_crew else None,
        is_nonrts=False,
        location=None,
        loc_key=None,
        parsed_construction=None,
    )
