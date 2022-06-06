from typing import List, Tuple, Dict

from .populate_db_data import LocationDbData

T_GROUPS_OF_LOCATIONS = Dict[Tuple, List[LocationDbData]]


def group_locations_by_key(rts_rows_with_location: List[LocationDbData]) -> T_GROUPS_OF_LOCATIONS:
    locations = {}
    for rv in rts_rows_with_location:
        if all([getattr(rv, attr) is None for attr in rv.__annotations__.keys() if attr != 'r']):
            continue

        k = (
            rv.r.address_legal.lower() if rv.r.address_legal is not None else None,
            rv.r.address_market.lower() if rv.r.address_market is not None else None,
            rv.postcode.district.title.lower() if rv.postcode is not None else None,
            rv.family_construction.title.lower() if rv.family_construction is not None else None,
            rv.r.cadastral_number,
        )
        try:
            locations[k].append(rv)
        except KeyError:
            locations[k] = [rv]

    for k, list_of_locations in locations.items():
        if len(list_of_locations) > 1:
            print(f'Error: length of location group ({len(list_of_locations)}) > 1: key: {k}')
            for _l in list_of_locations:
                print('--', _l)

    return locations
