from typing import List, Tuple, Dict

from .bulk_create_projects import T_BULK_PROJECTS, bulk_create_projects
from .create_ak import T_AK_CACHE
from .create_appendices import T_MAPPING_APPENDICES
from .utils import g_resolve_or_create, get_ak, g_resolve
from .._upload_projects_to_json.entities import UProject, UAppendix, UReservation, dataclass_to_dict


from ptc_deco.api import models as m


T_BULK_RESERVATIONS = List[Tuple[Tuple[UProject, m.Project], m.Reservation]]


def create_reservations(
    projects: T_BULK_PROJECTS, global_cache: Dict, appendices: T_MAPPING_APPENDICES, ak_cache: T_AK_CACHE
):
    app: UAppendix
    app_id: int
    u_project_obj: UProject
    model_project_obj: m.Project
    r: UReservation

    bulk_reservations: T_BULK_RESERVATIONS = []
    for u_project_obj, model_project_obj in projects:
        for r in u_project_obj.reservations:
            assert r.estimate_non_rts is None
            bulk_reservations.append(
                (
                    (u_project_obj, model_project_obj),
                    m.Reservation(
                        **dataclass_to_dict(r.base),
                        **dataclass_to_dict(
                            r,
                            {
                                'id',
                                'base',
                                'reservation_type',
                                'appendices',
                                'estimate_non_rts',
                                'agency_commission',
                                'construction_side',
                            },
                        ),
                        reservation_type=g_resolve_or_create(r.reservation_type, global_cache),
                        # estimate_non_rts=
                        agency_commission=get_ak(r, ak_cache),
                        construction_side=r.construction_side.resolve(u_project_obj, r, cache=global_cache)
                        if r.construction_side is not None
                        else None,
                        project=model_project_obj
                    ),
                )
            )

    if bulk_create_projects:
        to_create = [r for key, r in bulk_reservations]
        created = m.Reservation.objects.bulk_create(to_create)
        for ((u_project_obj, model_project_obj), r) in zip([key for key, r in bulk_reservations], created):
            for app in u_project_obj.project_appendices:
                appendices[app.id].reservations.add(r)
