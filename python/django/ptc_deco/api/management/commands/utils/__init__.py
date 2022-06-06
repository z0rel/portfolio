import sys
from collections import OrderedDict
from typing import Any, Dict

from .postcode_to_district_map import POSTCODE_TO_DISTRICT_MAP
from .read_yaml import read_yaml
from .SheetValue import SheetValue
from .GetOrCreate import GetOrCreate
from .TrSideTypes import TrSideTypes
from .get_families_tree import get_families_tree
from .get_family_objects import get_family_objects
from .update_db_obj_fields import set_row_str, set_row_db_id
from .get_default_country import get_default_country
from .get_sheet_value import get_sheet_value
from .action import Action
from .populate_cols_idx import populate_cols_idx
from .excel_base_names import (
    PATH_EXCEL_CONSTRUCTIONS,
    PATH_EXCEL_LOCATIONS,
    PATH_EXCEL_LOCATIONS_SHEET,
    PATH_EXCEL_CONSTRUCTIONS_SHEET_RTS,
    PATH_EXCEL_CONSTRUCTIONS_SHEET_NONRTS,
)


def print_row(row_idx, sheet):
    print('---', ' '.join([str(row_idx)] + [str(sheet.cell(row_idx, k).value) for k in range(1, sheet.max_column)]))


class NcntPrint:
    def __init__(self):
        self.ncnt = 0

    def __call__(self, val):
        if self.ncnt < 9:
            print(val, ' ', end='')
            sys.stdout.flush()
        else:
            print(val)
            self.ncnt = 0


def get_postcode(get_or_create, m, country, s_city, s_district, s_postcode=None):
    city = get_or_create(m.City, title=s_city, country_id=country.id) if s_city and country else None
    district = get_or_create(m.District, title=s_district, city_id=city.id) if city else None
    if s_postcode:
        s_postcode = s_postcode.strip()
    postcode = get_or_create(m.Postcode, title=s_postcode, district_id=district.id) if district else None
    return city, district, postcode


def filter_dict(d) -> Dict[str, Any]:
    return {k: v for (k, v) in d.items() if v is not None}


def filter_dict_ordered(d) -> OrderedDict:
    return OrderedDict([(k, v) for (k, v) in d if v is not None])


def generate_key(prefix, d):
    return prefix + ' '.join(f'{str(k)}: {str(v)}' for (k, v) in d.items())


def v2s(v):
    return '' if v is None else v
