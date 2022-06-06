from graphene.relay import Node

from .......api import models as m
from ....proto import sales_pb2
from ....utils import set_arg_isoformat, set_arg, base64_bytes, ContentFieldConnection, PageInfoApi

from .db_query import db_query_projects


STATIC_FIELDS = [
    'code',
    'num_in_year',
    'title',
    'client_title',
    'brand_title',
    'agency_title',
    'client_city_title',
    'back_office_manager_first_name',
    'back_office_manager_last_name',
    'sales_manager_first_name',
    'sales_manager_last_name',
]


def resolve_projects(parent, info, **kwargs):
    projects, offset, limit, count = db_query_projects(kwargs)

    result_projects_list = sales_pb2.Projects()
    for p in projects[offset:offset+limit]:
        project = result_projects_list.items.add()
        project.id = Node.to_global_id('VProjectOptimizedNode', p.id)

        set_arg_isoformat(project, 'start_date', p.start_date)
        set_arg_isoformat(project, 'created_at', p.created_at)
        for f in STATIC_FIELDS:
            set_arg(project, f, getattr(p, f))

        if p.working_sector_title:
            working_sector = project.working_sector.add()
            working_sector.id = "0"
            working_sector.title = p.working_sector_title

        if p.cities_list:
            for i, city_title in enumerate(p.cities_arr):
                if city_title:
                    city = project.cities.add()
                    city.id = str(i)
                    city.title = city_title

    result = ContentFieldConnection(
        content=base64_bytes(result_projects_list.SerializeToString()),
        pageInfo=PageInfoApi(total_count=count, offset=offset, limit=limit),
    )
    return result
