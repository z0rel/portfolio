from typing import List, Tuple, Dict, Optional, Union

from .._upload_projects_to_json.entities import UAgencyCommission, UProjectList
from ptc_deco.api import models as m


T_BULK_AK = List[Tuple[m.AgencyCommission, Tuple[str, int]]]
T_AK_CACHE = Dict[Tuple[str, int], m.AgencyCommission]


class ICacheKey:
    def cache_key(self) -> Tuple:
        pass


def append_bulk_ak(cache: T_AK_CACHE, bulk_ak: T_BULK_AK, obj: Union[object, ICacheKey], ak_attr: str):
    if obj is not None:
        ak_obj: Optional[UAgencyCommission] = getattr(obj, ak_attr)
        if ak_obj is not None:
            bulk_ak.append((ak_obj.create(cache), obj.cache_key()))


def bulk_create_ak(obj: UProjectList, global_cache: Dict) -> T_AK_CACHE:
    bulk_ak: T_BULK_AK = []
    ak_cache: T_AK_CACHE = {}

    for p in obj.project_list:
        append_bulk_ak(global_cache, bulk_ak, p, 'agency_commission')
        for r in p.reservations:
            append_bulk_ak(global_cache, bulk_ak, r, 'agency_commission')
        for add_rts in p.additional_costs:
            append_bulk_ak(global_cache, bulk_ak, add_rts, 'agency_commission')
        for add_nonrts in p.additional_costs_nonrts:
            append_bulk_ak(global_cache, bulk_ak, add_nonrts, 'agency_commission')

    if bulk_ak:
        to_create = [ak for ak, key in bulk_ak]
        keys = [key for ak, key in bulk_ak]
        ret = m.AgencyCommission.objects.bulk_create(to_create)
        for created_obj, key in zip(ret, keys):
            ak_cache[key] = created_obj

    return ak_cache
