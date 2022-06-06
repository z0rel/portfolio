import re
from functools import reduce
from typing import Optional
from django.db.models import Q, F, Case, When, Prefetch, QuerySet


RXP = re.compile(r'([0-9]*)(\.([0-9]*)(\.([A-Za-z0-9_-]*)?(\.([A-Za-z0-9_-]*)(\.([A-Za-z0-9_-]*))?)?)?)?')


class CodeMatch:
    def __init__(self, sidecode: str):
        m = RXP.match(sidecode)

        self.postcode__icontains = m.group(1)
        num_in_district = m.group(3)
        self.num_in_district = None if not num_in_district else int(num_in_district)
        self.code_format__icontains = m.group(5)
        self.code_side = m.group(7)
        self.code_adv_side = m.group(9)


def construction_side_code_to_filterspec(sidecode: str, prefix: str, from_reservation=False) -> Optional[Q]:
    if sidecode is None:
        return None

    m = CodeMatch(sidecode)

    print(f'postcode__icontains: {m.postcode__icontains}, num_in_district: {m.num_in_district}, '
          f'code_format__icontains: {m.code_format__icontains}, code_side: {m.code_side}, code_adv_side: {m.code_adv_side}')


    filterspec_dict = {}
    filterspec_dict_cs = {}
    filterspec_dict_s = {}
    if m.postcode__icontains:
        filterspec_dict_cs[f'construction_side__construction__location__postcode__title__icontains'] = m.postcode__icontains
        if not from_reservation:
            filterspec_dict[f'reservation__construction_side__construction__location__postcode__title__icontains'] = m.postcode__icontains
            filterspec_dict_s[f'construction__location__postcode__title__icontains'] = m.postcode__icontains
    if m.num_in_district is not None:
        filterspec_dict_cs[f'construction_side__construction__num_in_district'] = m.num_in_district
        if not from_reservation:
            filterspec_dict[f'reservation__construction_side__construction__num_in_district'] = m.num_in_district
            filterspec_dict_s[f'construction__num_in_district'] = m.num_in_district
    if m.code_format__icontains:
        filterspec_dict_cs[f'construction_side__advertising_side__side__format__code__icontains'] = m.code_format__icontains
        if not from_reservation:
            filterspec_dict[f'reservation__construction_side__advertising_side__side__format__code__icontains'] = m.code_format__icontains
            filterspec_dict_s[f'construction__formats__format__code__icontains'] = m.code_format__icontains
    if m.code_side:
        filterspec_dict_cs[f'construction_side__advertising_side__side__code__icontains'] = m.code_side
        if not from_reservation:
            filterspec_dict[f'reservation__construction_side__advertising_side__side__code__icontains'] = m.code_side
    if m.code_adv_side:
        filterspec_dict_cs[f'construction_side__advertising_side__code__icontains'] = m.code_adv_side
        if not from_reservation:
            filterspec_dict[f'reservation__construction_side__advertising_side__code__icontains'] = m.code_adv_side

    result = [x for x in [
        Q(**filterspec_dict) if filterspec_dict else None,
        Q(**filterspec_dict_cs) if filterspec_dict_cs else None,
        Q(**filterspec_dict_s) if filterspec_dict_s else None
    ] if x is not None]

    return reduce(lambda x, y: x | y, result) if result else None
