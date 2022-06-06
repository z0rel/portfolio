from typing import List

from ..utils import filter_dict
from .populate_partner_row_from_db_values import PartnerDbValues


def values_rv_id(rv_list: List[PartnerDbValues], attr: str):
    return set(
        [
            x
            for x in [getattr(rv, attr, None).id if getattr(rv, attr, None) is not None else None for rv in rv_list]
            if x is not None
        ]
    )


def values_r_str(rv_list: List[PartnerDbValues], attr: str):
    return set(
        [
            x
            for x in [getattr(rv.r, attr, None) if getattr(rv.r, attr, None) is not None else None for rv in rv_list]
            if x is not None
        ]
    )


def get_attr_and_check_its_unique(rv_list: List[PartnerDbValues], getter, attr):
    set_of_values = getter(rv_list, attr)
    if len(set_of_values) > 1:
        row_indices = ', '.join([str(x) for x in sorted(set([rv.r.row_idx for rv in rv_list]))])
        print(f'ERROR: Partner attr - {attr} is non unique for rows {row_indices}')
        for val in sorted(set_of_values):
            print('--', val)

    return list(set_of_values)[0] if set_of_values else None


def get_args_to_create_partner(rv_list: List[PartnerDbValues]):
    return filter_dict(
        {
            'bin_number': rv_list[0].r.bin_number,
            'title': rv_list[0].r.title,
            'partner_type_id': get_attr_and_check_its_unique(rv_list, values_rv_id, 'partner_type'),
            'client_type_id': get_attr_and_check_its_unique(rv_list, values_rv_id, 'client_type'),
            'actual_address_postcode_id': get_attr_and_check_its_unique(
                rv_list, values_rv_id, 'actual_address_postcode'
            ),
            'legal_address_postcode_id': get_attr_and_check_its_unique(rv_list, values_rv_id, 'legal_address_postcode'),
            'legal_address_id': get_attr_and_check_its_unique(rv_list, values_rv_id, 'legal_address'),
            'actual_address_id': get_attr_and_check_its_unique(rv_list, values_rv_id, 'actual_address'),
            'bank_recipient': get_attr_and_check_its_unique(rv_list, values_r_str, 'bank_recipient'),
            'checking_account': get_attr_and_check_its_unique(rv_list, values_r_str, 'checking_account'),
            'kbe': get_attr_and_check_its_unique(rv_list, values_r_str, 'kbe'),
        }
    )
