import sys
from itertools import chain
from typing import Any, Set, List, Dict, Tuple, Optional

from django.db.models import F

from ptc_deco.api import models as m
from tqdm import trange


def objects_by_title(objects, title_key) -> Dict[str, List]:
    objects = list(objects)
    res = {getattr(x, title_key): [] for x in objects}
    for x in objects:
        res[getattr(x, title_key)].append(x)
    return res


def objects_by_key_fn(objects, key_fn) -> Dict[Tuple, List]:
    res = {key_fn(x): [] for x in objects}
    for x in objects:
        res[key_fn(x)].append(x)
    return res


CHECK = True


def check_duplicates(entity, db_dict, check):
    has_duplicates = False
    if not check:
        return
    for k, v in db_dict.items():
        if len(v) > 1:
            has_duplicates = True
            print(f'Duplicated items of {entity}: k: {str(k)}, v: {v}')
    return has_duplicates


def merge_unexisted(key_fn, model_creator_fn, cached_objs: Set, m_class, db_dict, log: Any = False):
    """
    Создать существующие в cached_objs но несуществующие в db_dict значения в базу данных.
    Доббавить их db_dict.
    """
    _cached_objs = [s for s in cached_objs]
    if not _cached_objs:
        return

    is_structured = isinstance(_cached_objs[0], tuple) or isinstance(_cached_objs[0], list)
    if is_structured:
        _cached_objs_tr = set([x[0] for x in _cached_objs])
    else:
        _cached_objs_tr = set(_cached_objs)

    _unexisted_objs = _cached_objs_tr - set(db_dict.keys())
    if is_structured:
        _unexisted_objs = [x for x in _cached_objs if x[0] in _unexisted_objs]
    else:
        _unexisted_objs = list(_unexisted_objs)

    if log:
        print(log, _unexisted_objs)

    unexisted_objs = [m_class(**model_creator_fn(x)) for x in _unexisted_objs]
    if unexisted_objs:
        bulk_unexisted_objs = m_class.objects.bulk_create(unexisted_objs)
        for obj, cached_recrord in zip(bulk_unexisted_objs, _unexisted_objs):
            try:
                db_dict[key_fn(cached_recrord)].append(obj)
            except:
                db_dict[key_fn(cached_recrord)] = [obj]


def get_or_create_postcode_for_district(postcode_title, old_id, new_id, attr, obj):
    postcode = m.Postcode.objects.filter(title=postcode_title, district_id=new_id)
    need_update = True
    if postcode:
        postcode = postcode[0]
    else:
        existed_postcode = m.Postcode.objects.filter(title=postcode_title, district_id=old_id)
        if existed_postcode:
            need_update = False
            existed_postcode.update(district_id=new_id)
        else:
            postcode = m.Postcode(title=postcode_title, district_id=new_id)
            postcode.save()

    if need_update and obj is not None:
        setattr(obj, attr, postcode.id)
        obj.save(update_fields=[attr])
    return postcode.id, need_update


def get_or_create_address_for_district(
    address_title, postcode_title, old_district_id, new_district_id, attr, obj, old_postcode_id
):
    postcode_id, obj_need_update = get_or_create_postcode_for_district(
        postcode_title, old_district_id, new_district_id, None, None
    )
    if not obj_need_update:
        return

    address = m.Addresses.objects.filter(title=address_title, postcode_id=postcode_id)
    need_update = True
    if address:
        address = address[0]
    else:
        existed_address = m.Addresses.objects.filter(title=address_title, postcode_id=old_postcode_id)
        if existed_address:
            need_update = False
            existed_address.update(postcode_id=postcode_id)
        else:
            address = m.Addresses(address=address_title, postcode_id=postcode_id)
            address.save()

    if need_update:
        setattr(obj, attr, address.id)
        obj.save(update_fields=[attr])

    return address.id


def reduce_districts_everywhere(old_obj: m.District, new_district_id):
    old_district_id = old_obj.id
    m.Partner.objects.filter(district_id=old_district_id).update(district_id=new_district_id)
    p: m.Partner
    for p in m.Partner.objects.filter(legal_address_postcode__district_id=old_district_id).annotate(
        pc_title=F('legal_address_postcode__title'),
        address_pc_id=F('legal_address__postcode_id'),
        address_pc_title=F('legal_address__postcode__title'),
        address_title=F('legal_address__address'),
    ):
        get_or_create_postcode_for_district(
            p.pc_title, old_district_id, new_district_id, 'legal_address_postcode_id', p
        )
        # Partners (legal address)
        get_or_create_address_for_district(
            p.address_title,
            p.address_pc_title,
            old_district_id,
            new_district_id,
            'legal_address_id',
            p,
            p.address_pc_id,
        )

    iter_partners = m.Partner.objects.filter(actual_address_postcode__district_id=old_district_id).annotate(
        pc_title=F('actual_address_postcode__title'),
        address_pc_id=F('actual_address__postcode_id'),
        address_pc_title=F('actual_address__postcode__title'),
        address_title=F('actual_address__address'),
    )

    for p, _ in zip(iter_partners, trange(len(iter_partners), file=sys.stdout, descr='merge partners')):
        get_or_create_postcode_for_district(
            p.pc_title, old_district_id, new_district_id, 'actual_address_postcode_id', p
        )
        # Partners (actual address)
        get_or_create_address_for_district(
            p.address_title,
            p.address_pc_title,
            old_district_id,
            new_district_id,
            'actual_address_id',
            p,
            p.address_pc_id,
        )

    for loc in m.Location.objects.filter(postcode__district_id=old_district_id).annotate(
        pc_title=F('postcode__title'),
        m_address_pc_id=F('marketing_address__postcode_id'),
        m_address_pc_title=F('marketing_address__postcode__title'),
        m_address_title=F('marketing_address__address'),
        l_address_pc_id=F('legal_address__postcode_id'),
        l_address_pc_title=F('legal_address__postcode__title'),
        l_address_title=F('legal_address__address'),
    ):
        get_or_create_postcode_for_district(loc.pc_title, old_district_id, new_district_id, 'postcode_id', p)
        get_or_create_address_for_district(
            p.m_address_title,
            p.m_address_pc_title,
            old_district_id,
            new_district_id,
            'marketing_address_id',
            p,
            p.m_address_pc_id,
        )
        get_or_create_address_for_district(
            p.l_address_title,
            p.l_address_pc_title,
            old_district_id,
            new_district_id,
            'legal_address_id',
            p,
            p.l_address_pc_id,
        )

    # Удалить или обновить все оставшиеся почтовые коды
    # Addresses (postcode)
    for p in m.Addresses.objects.filter(postcode__district_id=old_district_id).annotate(
        pc_title=F('postcode__title'),
    ):
        get_or_create_postcode_for_district(p.pc_title, old_district_id, new_district_id, 'postcode_id', p)

    # Обновить оставшиеся почтовые коды
    m.Postcode.objects.filter(district_id=old_district_id).update(district_id=new_district_id)

    assert len(old_obj.postcodes) == 0 and len(old_obj.partners) == 0
    old_obj.delete()


TD_CITIES = Dict[str, List[m.City]]
TD_DISTRICTS = Dict[str, List[m.District]]
TD_DISTRICTS2 = Dict[Tuple[type(m.District.title), type(m.District.city_id)], List[m.District]]
TD_POSTCODES = Dict[str, List[m.Postcode]]
TD_POSTCODES2 = Dict[
    Tuple[type(m.Postcode.title), type(m.Postcode.district_id), type(m.District.city_id)], List[m.Postcode]
]


def merge_city_district_postcode(
    cached_xlsx_rows, country: m.Country
) -> Tuple[TD_CITIES, TD_DISTRICTS, TD_DISTRICTS2, TD_POSTCODES, TD_POSTCODES2]:
    cached_cities = set([r.s_city for r in cached_xlsx_rows])

    # _al = m.City.objects.filter(title='Алматы')
    # if _al:
    #     _al[0].delete()
    db_cities: TD_CITIES = objects_by_title(m.City.objects.filter(title__in=cached_cities).only('id', 'title'), 'title')
    check_duplicates('Before merge: City', db_cities, CHECK)

    merge_unexisted(lambda x: x, lambda x: {'title': x, 'country': country}, cached_cities, m.City, db_cities)

    IDX_DISTRICT_TITLE = 0
    IDX_DISTRICT_CITY = 1
    cached_districts = set([(r.s_district, r.s_city) for r in cached_xlsx_rows])
    # _al = m.District.objects.filter(title__in={'Медеуский', 'Алматы'})
    # if _al:
    #     for a in _al:
    #         a.delete()

    db_districts: TD_DISTRICTS

    def get_districts() -> Tuple[TD_DISTRICTS, TD_DISTRICTS2]:
        _db_districts = objects_by_title(
            m.District.objects.filter(title__in=set([x[0] for x in cached_districts])), 'title'
        )
        _db_districts2 = objects_by_key_fn(
            m.District.objects.filter(title__in=set([x[0] for x in cached_districts])), lambda x: (x.title, x.city_id)
        )
        return _db_districts, _db_districts2

    db_districts, db_districts2 = get_districts()
    check_duplicates('Before merge: District', db_districts2, CHECK)
    merge_unexisted(
        lambda x: x[IDX_DISTRICT_TITLE],
        lambda x: {'title': x[IDX_DISTRICT_TITLE], 'city': db_cities[x[IDX_DISTRICT_CITY]][0]},
        cached_districts,
        m.District,
        db_districts,
    )

    for k, v in db_districts2.items():
        if len(v) > 1:
            base_id = v[0].id
            for obj in v[1:]:
                reduce_districts_everywhere(obj, base_id)

    db_districts, db_districts2 = get_districts()

    # _al = m.Postcode.objects.filter(title__in={'010003', '050006'})
    # if _al:
    #     for a in _al:
    #         a.delete()

    cached_postcodes = set(
        [(r.s_postcode, r.s_district, r.s_city) for r in cached_xlsx_rows if (r.s_postcode is not None)]
    )
    db_postcodes: TD_POSTCODES = objects_by_title(
        m.Postcode.objects.filter(title__in=set([x[0] for x in cached_postcodes])), 'title'
    )
    db_postcodes2: TD_POSTCODES2 = objects_by_key_fn(
        m.Postcode.objects.filter(title__in=set([x[0] for x in cached_postcodes])),
        lambda x: (x.title, x.district_id, x.district.city_id),
    )

    def model_creator_fn(x: Tuple[str, Optional[str], Optional[str]]):  # почтовый код, район, город
        title = x[0]
        k1 = x[1]
        k2 = db_districts[k1]
        district = k2[0]
        return {'title': title, 'district': district}

    check_duplicates('Before merge: Postcode (merge_city_district_postcode)', db_postcodes, CHECK)
    merge_unexisted(
        lambda x: x[0],
        model_creator_fn,
        cached_postcodes,
        m.Postcode,
        db_postcodes,
    )

    check_duplicates('After merge: City', db_cities, CHECK)
    check_duplicates('After merge: District', db_districts2, CHECK)
    check_duplicates('After merge: Postcode', db_postcodes, CHECK)

    return db_cities, db_districts, db_districts2, db_postcodes, db_postcodes2
