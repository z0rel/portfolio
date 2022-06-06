from typing import List, Dict, Tuple

from django.db.models import Q

from .populate_rts_row_db_values import ConstructionRtsRowDbValues
from ..utils import filter_dict_ordered, filter_dict, generate_key

from ptc_deco.api import models as m


def load_or_create_location_for_construction_rts(
    db_row: ConstructionRtsRowDbValues,
    locations_by_cadastral_number: Dict[Tuple, List[m.Location]],
    locations_by_partial_key: Dict[Tuple, List[m.Location]],
    bulk_create_locations: List[Tuple[m.Location, ConstructionRtsRowDbValues]],
    locations_to_create_by_key: Dict
):

    if db_row.r.cadastral_number:
        partial_key = (
            db_row.r.cadastral_number,
            db_row.r.s_address_market.lower() if db_row.r.s_address_market is not None else None,
        )
        by_cadastral_number = locations_by_cadastral_number.get(partial_key, None)
        if by_cadastral_number:
            if len(by_cadastral_number) > 1:
                print(
                    f"""Error: construction rts row: {db_row.r.row_idx}, location by cadastral number {
                       db_row.r.cadastral_number} is non unique: length = {len(by_cadastral_number)}"""
                )
                for _l in by_cadastral_number:
                    print('--', _l)

            db_row.location = by_cadastral_number[0]
            return
    else:  # если кадастровый номер не задан - поискать местоположение по частичному совпадению ключа
        partial_key = (
            db_row.r.s_address_legal.lower() if db_row.r.s_address_legal is not None else None,
            db_row.r.s_address_market.lower() if db_row.r.s_address_market is not None else None,
            db_row.district.id if db_row.district is not None else None,
            db_row.family_construction.id if db_row.family_construction is not None else None,
        )
        by_partial_key = locations_by_partial_key.get(partial_key, None)
        if by_partial_key:
            if len(by_partial_key) > 1:
                print(
                    f"""Error: construction rts row: {db_row.r.row_idx}, location by partial key {
                by_partial_key} is non unique: length = {len(by_partial_key)}"""
                )
                for _l in by_partial_key:
                    print('--', _l)

                db_row.location = by_partial_key[0]
                return

    # Если ничего по кадастровому номеру или частичному ключевому совпадению не найдено - создать местоположение
    created_partial_key = (
        db_row.legal_address.id if db_row.legal_address is not None else None,
        db_row.marketing_address.id if db_row.marketing_address is not None else None,
        db_row.postcode.id if db_row.postcode is not None else None,
        db_row.family_construction.id if db_row.family_construction is not None else None,
        db_row.r.cadastral_number
    )

    args_location_create = filter_dict(
        {
            'construction_row_idx': db_row.r.row_idx,
            'legal_address': db_row.legal_address,
            'marketing_address': db_row.marketing_address,
            'postcode': db_row.postcode,
            'family_construction': db_row.family,
            'cadastral_number': db_row.r.cadastral_number,
        }
    )
    # Местоположение создается, если заданы хотя бы адрес или кадастровый номер
    if (
        'legal_address' in args_location_create
        or 'marketing_address' in args_location_create
        or 'cadastral_number' in args_location_create
    ):
        if created_partial_key not in locations_to_create_by_key:
            location = m.Location(has_area=db_row.has_area, **args_location_create)
            locations_to_create_by_key[created_partial_key] = {'location': location, 'rows': [db_row]}
            bulk_create_locations.append([location, created_partial_key])
        else:
            locations_to_create_by_key[created_partial_key]['rows'].append(db_row)

