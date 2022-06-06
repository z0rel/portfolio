from typing import List, Tuple, Dict

from .bulk_create_projects import T_BULK_PROJECTS
from .utils import g_resolve
from .._upload_projects_to_json.entities import UProject, UAppendix, dataclass_to_dict
from ptc_deco.api import models as m


T_BULK_APPENDICES = List[Tuple[Tuple[UProject, m.Project, UAppendix], m.Appendix]]
T_MAPPING_APPENDICES = Dict[int, m.Appendix]


def create_appendices(projects: T_BULK_PROJECTS, global_cache: Dict) -> Tuple[T_BULK_APPENDICES, T_MAPPING_APPENDICES]:
    app: UAppendix
    app_id: int
    u_project_obj: UProject
    model_project_obj: m.Project

    bulk_appendices: T_BULK_APPENDICES = []
    for u_project_obj, model_project_obj in projects:
        for app_id, app in u_project_obj.project_appendices.items():
            bulk_appendices.append(
                (
                    (u_project_obj, model_project_obj, app),
                    m.Appendix(
                        **dataclass_to_dict(app, {'id', 'creator', 'sales_manager'}),
                        creator=g_resolve(app.creator, global_cache),
                        sales_manager=g_resolve(app.sales_manager, global_cache),
                        project=model_project_obj,
                    ),
                )
            )
    if bulk_appendices:
        to_create = [app for _, app in bulk_appendices]
        created = m.Appendix.objects.bulk_create(to_create)
        bulk_appendices = [(key, cr) for ((key, _), cr) in zip(bulk_appendices, created)]

    mapping_ids = {}
    model_appendix: m.Appendix
    for (u_project_obj, model_project_obj, app), model_appendix in bulk_appendices:
        mapping_ids[app.id] = model_appendix

    return bulk_appendices, mapping_ids
