from typing import List, Tuple, Dict

from .create_ak import T_AK_CACHE
from .utils import g_resolve_or_update, g_resolve, get_ak
from .._upload_projects_to_json.entities import UProject, UProjectList, dataclass_to_dict

from ptc_deco.api import models as m

T_BULK_PROJECTS = List[Tuple[UProject, m.Project]]


def bulk_create_projects(obj: UProjectList, ak_cache: T_AK_CACHE, global_cache: Dict) -> T_BULK_PROJECTS:
    # 2. Создать проекты и приложения, сделать маппинг приложений из старых id в новые
    bulk_projects = [
        m.Project(
            **dataclass_to_dict(
                p,
                {
                    'id',
                    'brand',
                    'client',
                    'agency',
                    'creator',
                    'back_office_manager',
                    'sales_manager',
                    'reservations',
                    'additional_costs_nonrts',
                    'additional_costs',
                    'project_appendices',
                    'agency_commission',
                },
            ),
            brand=g_resolve_or_update(p.brand, global_cache),
            client=g_resolve(p.client, global_cache),
            agency=g_resolve(p.agency, global_cache),
            creator=g_resolve(p.creator, global_cache),
            back_office_manager=g_resolve(p.back_office_manager, global_cache),
            sales_manager=g_resolve(p.sales_manager, global_cache),
            agency_commission=get_ak(p, ak_cache),
        )
        for p in obj.project_list
    ]
    if bulk_projects:
        created_projects = m.Project.objects.bulk_create(bulk_projects)
        return list(zip(obj.project_list, created_projects))
    return []
