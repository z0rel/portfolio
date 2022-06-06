from collections import namedtuple
from itertools import chain
from typing import List, Union, Dict, Optional, Tuple, Set

from tqdm import tqdm

from django.db.models import F
from ptc_deco.api import models as m

from .populate_rts_row_from_sheet import ConstructionRtsStrRow
from .merge_city_district_postcode import objects_by_title, TD_POSTCODES
from ..utils import get_postcode

IDX_ADDR_TITLE = 0
IDX_POSTCODE_TITLE = 1


def append_or_create(dict_item, key_address, key_postcode, key_district, key_city, val):
    try:
        subval_postcode = dict_item[key_address]
    except KeyError:
        subval_postcode = {}
        dict_item[key_address] = subval_postcode

    key = (key_postcode, key_district, key_city)

    try:
        subval_postcode[key].append(val)
    except KeyError:
        subval_postcode[key] = [val]


class AnnotationMergedAddress:
    postcode_title: Optional[type(m.Postcode.title)]
    district_title: Optional[type(m.District.title)]
    city_title: Optional[type(m.City.title)]
    district_id: Optional[int]
    city_id: Optional[int]
    country_id: Optional[int]
    district: Optional[m.District]
    city: Optional[m.City]


T_DB_ADDRESS = Union[m.Addresses, AnnotationMergedAddress]
T_DB_ADDRESSES = List[T_DB_ADDRESS]


def get_db_addresses(cached_addresses) -> T_DB_ADDRESSES:
    return (
        m.Addresses.objects.filter(address__in=cached_addresses)
        .select_related('postcode')
        .annotate(
            postcode_title=F('postcode__title'),
            district=F('postcode__district'),
            district_id=F('postcode__district_id'),
            district_title=F('postcode__district__title'),
            city_title=F('postcode__district__city__title'),
            city_id=F('postcode__district__city_id'),
            city=F('postcode__district__city'),
            country_id=F('postcode__district__city__country_id'),
        )
    )


def populate_postcode_lists(addr, addr_objs):
    address_with_postcode_none = []
    address_with_postcode_empty = []
    address_with_postcode_nsnonrts = []
    address_with_postcode_nonempty = []

    for addr_obj in addr_objs:
        postcode_title = addr_obj.postcode_title
        if postcode_title is None:
            address_with_postcode_none.append((addr, addr_obj))
        elif postcode_title == '':
            address_with_postcode_empty.append(
                (addr, addr_obj, (postcode_title, addr_obj.district_title, addr_obj.city_title))
            )
        elif postcode_title[-6:] == 'NONRTS':
            address_with_postcode_nsnonrts.append(
                (addr, addr_obj, (postcode_title, addr_obj.district_title, addr_obj.city_title))
            )
        else:
            address_with_postcode_nonempty.append(
                (addr, addr_obj, (postcode_title, addr_obj.district_title, addr_obj.city_title))
            )

    weak_addr_for_none = (
        address_with_postcode_nonempty[0]
        if address_with_postcode_nonempty
        else address_with_postcode_empty[0]
        if address_with_postcode_empty
        else None
    )

    return (
        address_with_postcode_none,
        address_with_postcode_empty,
        address_with_postcode_nsnonrts,
        address_with_postcode_nonempty,
        weak_addr_for_none,
    )


def filter_postcode_empty_predicate(s_district, s_city, address_with_postcode_nonempty):
    for (it_addr, it_addr_obj, (it_s_postcode, it_s_district, it_s_city)) in address_with_postcode_nonempty:
        if s_district == it_s_district and s_city == it_s_city:
            return it_addr_obj.id

    return None


def reduce_model_postcodes_on_addresses(db_addresses, IDX_ADDR_OBJ, lambda_none, lambda_empty, lambda_nonrts):
    for addr, addr_objs in db_addresses.items():
        (
            address_with_postcode_none,
            address_with_postcode_empty,
            address_with_postcode_nsnonrts,
            address_with_postcode_nonempty,
            weak_addr_for_none,
        ) = populate_postcode_lists(addr, addr_objs)

        # Редукция элементов с почтовыми кодами None
        if weak_addr_for_none:
            fnd = weak_addr_for_none[IDX_ADDR_OBJ]
            for (addr, addr_obj) in address_with_postcode_none:
                if fnd and addr_obj.id != fnd:
                    lambda_none(addr_obj, fnd)

        # Редукция элементов с почтовыми кодами ''
        if address_with_postcode_nonempty:
            for (addr, addr_obj, (s_postcode, s_district, s_city)) in address_with_postcode_empty:
                fnd = filter_postcode_empty_predicate(s_district, s_city, address_with_postcode_nonempty)
                if fnd is not None and addr_obj.id != fnd:
                    lambda_empty(addr_obj, fnd)

        # Редукция элементов с почтовыми кодами 'NSNONRTS'
        if address_with_postcode_nonempty or address_with_postcode_empty:
            for (addr, addr_obj, (s_postcode, s_district, s_city)) in address_with_postcode_nsnonrts:
                fnd = filter_postcode_nonrts_predicate(
                    s_district, s_city, address_with_postcode_nonempty, address_with_postcode_empty
                )

                if fnd is not None and addr_obj.id != fnd:
                    lambda_nonrts(addr_obj, fnd)


def filter_postcode_nonrts_predicate(s_district, s_city, address_with_postcode_nonempty, address_with_postcode_empty):
    pred = preidcate_district_city if s_district != '' and s_district is not None else preidcate_city
    for (it_addr, it_addr_obj, (it_s_postcode, it_s_district, it_s_city)) in chain(
        address_with_postcode_nonempty, address_with_postcode_empty
    ):
        if pred(s_district, it_s_district, s_city, it_s_city):
            return it_addr_obj.id
    return None


# str - address: { список аннотированных объектов }
def merge_addresses(
    cached_xlsx_rows: List[ConstructionRtsStrRow], db_postcodes: TD_POSTCODES, country, get_or_create
) -> Dict[str, T_DB_ADDRESSES]:
    # (key_postcode, key_district, key_city)
    cached_addresses: Dict[str, Dict[Tuple[str, str, str], List[ConstructionRtsStrRow]]] = {}

    for r in cached_xlsx_rows:
        if r.s_address_legal == r.s_address_market and r.s_address_market is not None:
            append_or_create(cached_addresses, r.s_address_legal, r.s_postcode, r.s_district, r.s_city, r)
        else:
            if r.s_address_legal is not None:
                append_or_create(cached_addresses, r.s_address_legal, r.s_postcode, r.s_district, r.s_city, r)
            if r.s_address_market is not None:
                append_or_create(cached_addresses, r.s_address_market, r.s_postcode, r.s_district, r.s_city, r)

    def get_db_address_objs() -> Dict[str, T_DB_ADDRESSES]:
        return objects_by_title(get_db_addresses(cached_addresses), 'address')

    existed_db_addresses = get_db_address_objs()
    existed_addr_set = set(existed_db_addresses.keys())

    # Создать несуществующие адреса
    unexisted_addr = set(cached_addresses.keys()) - existed_addr_set
    bulk_address_unexisted = []
    for addr in unexisted_addr:
        objs = cached_addresses[addr]
        for (key_postcode, key_district, key_city) in objs.keys():
            city, district, postcode = get_postcode(get_or_create, m, country, key_city, key_district, key_postcode)
            bulk_address_unexisted.append(m.Addresses(address=addr, postcode=postcode))

    m.Addresses.objects.bulk_create(bulk_address_unexisted)

    existed_db_addresses = get_db_address_objs()

    # Извлечение несуществующих адресов для пакетного создания
    bulk_create_address: Set[Tuple[str, str]] = set()
    for addr, addr_objs in existed_db_addresses.items():
        postcode_to_existed_address_objects = {}

        for addr_obj in addr_objs:
            postcode_title = addr_obj.postcode_title
            if postcode_title is None or postcode_title == '' or postcode_title[-6:] == 'NONRTS':
                continue
            else:
                _k = (postcode_title, addr_obj.district_title, addr_obj.city_title)
                try:
                    postcode_to_existed_address_objects[_k].append((addr, addr_obj))
                except KeyError:
                    postcode_to_existed_address_objects[_k] = [(addr, addr_obj)]

        # Создать несуществующие адреса, которые с теми же именами есть в других районах, но не созданы для заданного
        for (k_postcode, k_district, k_city), r_list in cached_addresses[addr].items():
            if (k_postcode, k_district, k_city) not in postcode_to_existed_address_objects:
                bulk_create_address.add((addr, k_postcode))

    existed_bulk_addressed_set = set(
        [
            (x.address, x.postcode_id)
            for x in m.Addresses.objects.filter(address__in=set([x[IDX_ADDR_TITLE] for x in bulk_create_address]))
        ]
    )

    bulk_create_address_d = [
        m.Addresses(address=x[IDX_ADDR_TITLE], postcode=db_postcodes[x[IDX_POSTCODE_TITLE]][0])
        for x in bulk_create_address
        if (x[IDX_ADDR_TITLE], db_postcodes[x[IDX_POSTCODE_TITLE]][0].id) not in existed_bulk_addressed_set
    ]
    m.Addresses.objects.bulk_create(bulk_create_address_d)

    db_addresses = get_db_address_objs()

    IDX_ADDR_OBJ = 1
    counters = [0, 0, 0]

    def inc_counter(idx):
        counters[idx] += 1

    def handle_postcode_update(addr_obj, new_addr_obj_id, progress):
        progress.update(1)
        reduce_address_on_all_models(addr_obj, new_addr_obj_id)

    reduce_model_postcodes_on_addresses(
        db_addresses,
        IDX_ADDR_OBJ,
        (lambda _, __: inc_counter(0)),
        (lambda _, __: inc_counter(1)),
        (lambda _, __: inc_counter(2)),
    )

    with tqdm(total=counters[0], position=1, desc='postcodes None') as progress_none, tqdm(
        total=counters[1], position=2, desc='postcodes Empty'
    ) as progress_empty, tqdm(total=counters[2], position=3, desc='postcodes Nonrts') as progress_nonrts:

        reduce_model_postcodes_on_addresses(
            db_addresses,
            IDX_ADDR_OBJ,
            (lambda addr, id_val: handle_postcode_update(addr, id_val, progress_none)),
            (lambda addr, id_val: handle_postcode_update(addr, id_val, progress_empty)),
            (lambda addr, id_val: handle_postcode_update(addr, id_val, progress_nonrts)),
        )

    return get_db_address_objs()


def preidcate_district_city(s_district, it_s_district, s_city, it_s_city):
    return s_district == it_s_district and s_city == it_s_city


def preidcate_city(s_district, it_s_district, s_city, it_s_city):
    return s_city == it_s_city


def reduce_address_on_all_models(old_obj, new_id):
    old_id = old_obj.id
    m.Location.objects.filter(marketing_address_id=old_id).update(marketing_address_id=new_id)
    m.Location.objects.filter(legal_address_id=old_id).update(legal_address_id=new_id)
    m.Partner.objects.filter(legal_address_id=old_id).update(legal_address_id=new_id)
    m.Partner.objects.filter(actual_address_id=old_id).update(actual_address_id=new_id)
    old_obj.delete()
