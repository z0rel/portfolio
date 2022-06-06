from typing import Optional, Union, Tuple, Dict, List

from django.db.models import F

from ptc_deco.api import models as m


class AnnotatedAddress:
    city: Optional[m.City]
    district: Optional[m.District]
    city_title: Optional[str]
    district_title: Optional[str]
    postcode_title: Optional[str]


T_ANNOTATED_ADDRESSES = Union[m.Addresses, AnnotatedAddress]

# key: address, city_title, values - list of address
T_DICT_DB_ADDRESSES = Dict[Tuple[str, Optional[str]], List[T_ANNOTATED_ADDRESSES]]


def get_annotated_all_addresses() -> T_DICT_DB_ADDRESSES:
    addresses_by_address: T_DICT_DB_ADDRESSES = {}

    a: T_ANNOTATED_ADDRESSES
    for a in m.Addresses.objects.all().annotate(
        city=F('postcode__district__city'),
        district=F('postcode__district'),
        city_title=F('postcode__district__city__title'),
        district_title=F('postcode__district__title'),
        postcode_title=F('postcode__title'),
    ):

        key: Tuple[str, Optional[str]] = (
            a.address.lower(),
            a.city_title.lower() if a.city_title is not None else None,
        )
        try:
            addresses_by_address[key].append(a)
        except KeyError:
            addresses_by_address[key] = [a]

    return addresses_by_address
