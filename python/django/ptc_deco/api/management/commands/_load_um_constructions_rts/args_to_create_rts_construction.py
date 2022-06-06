from dataclasses import dataclass
from typing import List, Dict, Set, Optional

from .populate_rts_row_db_values import ConstructionRtsRowDbValues
from ..utils import filter_dict

from ptc_deco.api import models as m


def get_set_of_attr(rv_list: List[ConstructionRtsRowDbValues], attr: str) -> Set[int]:
    return set([getattr(rv, attr).id for rv in rv_list if getattr(rv, attr) is not None])


def get_set_of_attr_str(rv_list: List[ConstructionRtsRowDbValues], attr: str) -> Set[int]:
    return set([getattr(rv, attr) for rv in rv_list if getattr(rv, attr) is not None])


def get_set_of_attr_r_str(rv_list: List[ConstructionRtsRowDbValues], attr: str) -> Set[int]:
    return set([getattr(rv.r, attr) for rv in rv_list if getattr(rv.r, attr) is not None])


def get_attr_and_check(rv_list: List, attr: str, model, getter=None) -> List:
    getter = getter if getter is not None else (get_set_of_attr if model is not None else get_set_of_attr_str)
    ids = list(sorted(getter(rv_list, attr)))

    if len(ids) > 1:
        print(
            f"""ERROR: {attr} for construction with row_idx {rv_list[0].r.row_idx} and oto number {
            rv_list[0].r.tech_invent_number} is non unique. Count of {attr}s: {
            len(ids)}"""
        )
        if model is not None:
            for _l in ids:
                print('--', list(model.objects.filter(id=_l)))
        else:
            for _l in ids:
                print('--', _l)
    return ids


def get_phones(rv_list: List[ConstructionRtsRowDbValues]) -> Optional[str]:
    return (
        ';'.join(
            sorted(
                set([str(rv.r.tech_phone_construction) for rv in rv_list if rv.r.tech_phone_construction is not None])
            )
        )
        or None
    )


@dataclass
class AggregatedConstructionDataToCreateOrUpdate:
    tech_phone_construction: Optional[str]
    status_connection: bool
    crews: Dict[int, m.Crew]
    crew_id: Optional[int]
    locations: List[int]
    coordinates: Optional[str]
    crews_has_special_for_sides: bool
    model_id: Optional[int]
    location_id: Optional[int]

    marketing_addresses: List[int]
    marketing_address_id: Optional[int]

    legal_addresses: List[int]
    legal_address_id: Optional[int]

    postcodes: List[int]
    postcode_id: Optional[int]

    tech_invent_numbers: List[str]
    tech_invent_number: Optional[str]

    buh_invent_numbers: List[str]
    buh_invent_number: Optional[str]
    is_nonrts: bool


def aggregate_construction_data_to_create_or_update(rv_list: List[ConstructionRtsRowDbValues]):
    crews: Dict[int, m.Crew] = {rv.crew.id: rv.crew for rv in rv_list if rv.crew is not None}

    crew_id = list(sorted(crews.values(), key=lambda x: (x.name, x.id)))[0].id if crews else None
    models = get_attr_and_check(rv_list, 'model', m.ModelConstruction)
    locations = get_attr_and_check(rv_list, 'location', m.Location)

    marketing_addresses: List[int] = get_attr_and_check(rv_list, 'marketing_address', m.Addresses)
    marketing_address_id: Optional[int] = marketing_addresses[0] if marketing_addresses else None

    legal_addresses: List[int] = get_attr_and_check(rv_list, 'legal_address', m.Addresses)
    legal_address_id: Optional[int] = legal_addresses[0] if legal_addresses else None

    postcodes: List[int] = get_attr_and_check(rv_list, 'postcode', m.Postcode)
    postcode_id: Optional[int] = postcodes[0] if postcodes else None

    tech_invent_numbers: List[str] = get_attr_and_check(rv_list, 'tech_invent_number', None, get_set_of_attr_r_str)
    tech_invent_number: Optional[str] = tech_invent_numbers[0] if tech_invent_numbers else None

    buh_invent_numbers: List[str] = get_attr_and_check(rv_list, 'buh_invent_number', None, get_set_of_attr_r_str)
    buh_invent_number: Optional[str] = buh_invent_numbers[0] if buh_invent_numbers else None

    return AggregatedConstructionDataToCreateOrUpdate(
        tech_phone_construction=get_phones(rv_list),
        status_connection=all([rv.status_connection for rv in rv_list]),
        crews=crews,
        crew_id=crew_id,
        model_id=models[0],
        crews_has_special_for_sides=len(crews) > 1,
        locations=locations,
        location_id=list(sorted(locations))[0] if locations else None,
        coordinates=(
            '; '.join(sorted(set([rv.r.coordinates for rv in rv_list if rv.r.coordinates is not None]))) or None
        ),
        marketing_addresses=marketing_addresses,
        marketing_address_id=marketing_address_id,
        legal_addresses=legal_addresses,
        legal_address_id=legal_address_id,
        postcodes=postcodes,
        postcode_id=postcode_id,
        tech_invent_numbers=tech_invent_numbers,
        tech_invent_number=tech_invent_number,
        buh_invent_numbers=buh_invent_numbers,
        buh_invent_number=buh_invent_number,
        is_nonrts=False
    )


def get_args_to_create_rts_construction(rv_list: List[ConstructionRtsRowDbValues]):
    rv = rv_list[0]

    agg: AggregatedConstructionDataToCreateOrUpdate = aggregate_construction_data_to_create_or_update(rv_list)

    return filter_dict(
        {
            'row_idx': rv.r.row_idx,
            'tech_invent_number': agg.tech_invent_number,
            'buh_invent_number': agg.buh_invent_number,
            'tech_phone_construction': agg.tech_phone_construction,
            'status_connection': agg.status_connection,
            'model_id': agg.model_id,
            'crew_id': agg.crew_id,
            'crews_has_special_for_sides': agg.crews_has_special_for_sides,
            'location_id': agg.location_id,
            'is_nonrts': False,
            'coordinates': agg.coordinates,
            'marketing_address_id': agg.marketing_address_id,
            'legal_address_id': agg.legal_address_id,
            'postcode_id': agg.postcode_id,
        }
    )
