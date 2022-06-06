from typing import Dict


from .bulk_create_projects import T_BULK_PROJECTS, bulk_create_projects
from .create_ak import T_AK_CACHE, bulk_create_ak
from .create_appendices import T_BULK_APPENDICES, T_MAPPING_APPENDICES, create_appendices
from .create_reservations import create_reservations
from .utils import g_resolve, get_ak
from .._upload_projects_to_json.entities import (
    UProjectList,
    UProject,
    UEstimateNonRts,
    dataclass_to_dict,
    UAdditionalCosts,
)

from ptc_deco.api import models as m


def create_estimate_nonrts(
    projects: T_BULK_PROJECTS, global_cache: Dict, appendices: T_MAPPING_APPENDICES, ak_cache: T_AK_CACHE
):
    bulk_estimate_nonrts = []
    u_project_obj: UProject
    model_project_obj: m.Project
    obj: UEstimateNonRts

    for u_project_obj, model_project_obj in projects:
        for obj in u_project_obj.additional_costs_nonrts:
            bulk_estimate_nonrts.append(
                (
                    (u_project_obj, model_project_obj, obj),
                    m.EstimateNonRts(
                        **dataclass_to_dict(obj, {'id', 'appendix', 'agency_commission', 'city'}),
                        city=g_resolve(obj.city, global_cache),
                        agency_commission=get_ak(obj, ak_cache),
                        project=model_project_obj
                    ),
                )
            )
    if bulk_estimate_nonrts:
        to_create = [cr for (proj, cr) in bulk_estimate_nonrts]
        created = m.EstimateNonRts.objects.bulk_create(to_create)
        for (u_project_obj, model_project_obj, obj), model_estimate_non_rts in zip(
            [proj for (proj, cr) in bulk_estimate_nonrts], created
        ):
            for app in obj.appendix:
                appendices[app].additional_costs_nonrts.add(model_estimate_non_rts)


def create_additional_costs(
    projects: T_BULK_PROJECTS, global_cache: Dict, appendices: T_MAPPING_APPENDICES, ak_cache: T_AK_CACHE
):
    bulk_additional_costs = []
    u_project_obj: UProject
    model_project_obj: m.Project
    obj: UAdditionalCosts

    for u_project_obj, model_project_obj in projects:
        for obj in u_project_obj.additional_costs:
            bulk_additional_costs.append(
                (
                    (u_project_obj, model_project_obj, obj),
                    m.AdditionalCosts(
                        **dataclass_to_dict(obj, {'id', 'agency_commission', 'city', 'appendix'}),
                        agency_commission=get_ak(obj, ak_cache),
                        city=g_resolve(obj.city, global_cache),
                        project=model_project_obj
                    ),
                )
            )

    if bulk_additional_costs:
        to_create = [cr for (proj, cr) in bulk_additional_costs]
        created = m.EstimateNonRts.objects.bulk_create(to_create)
        for (u_project_obj, model_project_obj, obj), model_additional_costs in zip(
            [proj for (proj, cr) in bulk_additional_costs], created
        ):
            for app in obj.appendix:
                appendices[app].additional_costs_nonrts.add(model_additional_costs)


def load_model_from_json(obj: UProjectList):
    global_cache = {}
    # 1. Создать агентскую комиссию для объектов
    ak_cache: T_AK_CACHE = bulk_create_ak(obj, global_cache)
    projects: T_BULK_PROJECTS = bulk_create_projects(obj, ak_cache, global_cache)
    bulk_appendices: T_BULK_APPENDICES
    mapping_appendices: T_MAPPING_APPENDICES
    bulk_appendices, mapping_appendices = create_appendices(projects, global_cache)
    create_reservations(projects, global_cache, mapping_appendices, ak_cache)
    create_estimate_nonrts(projects, global_cache, mapping_appendices, ak_cache)
    create_additional_costs(projects, global_cache, mapping_appendices, ak_cache)
