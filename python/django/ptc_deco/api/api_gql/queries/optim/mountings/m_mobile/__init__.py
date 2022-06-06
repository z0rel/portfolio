# import sqlparse
from django.http import JsonResponse

from .......api import models as m
from ....utils import ContentJSONFieldConnection, PageInfoApi
from .graphql_answer_entities import MountingDesignField, MountingCrewField, MountingProjectField, ConstructionSideInfo
from .db_query import db_query
from .populate_ordered_dict_common import get_construction_side_code, populate_ordered_dict_common

from .graphql_answer_main import MountingField, MOUNTING_RANGE_ENUM
from .graphql_query_field import MountingFieldConnection
from .graphql_query_field import mountings_debug_field, mountings_field


def resolve_mountings_common(parent, info, to_json, **kwargs):
    copied_kwargs = {**kwargs}
    mountings, offset, limit, count, order_by_list = db_query(copied_kwargs)

    v: m.Mounting
    result = []
    for v in mountings:
        code = get_construction_side_code(v)
        task_name = f'Монтаж {code}' if v.mounting_task_title is None else f'{v.mounting_task_title} - {code}'
        item = populate_ordered_dict_common(v, task_name, to_json, code)

        item['name'] = task_name
        item['isCommonTask'] = bool(v.mounting_task_title)
        result.append(item)

    return count, offset, limit, result


def resolve_mountings(parent, info, **kwargs):
    count, offset, limit, result = resolve_mountings_common(parent, info, True, **kwargs)

    response = JsonResponse(result, safe=False, json_dumps_params={'ensure_ascii': False})
    return ContentJSONFieldConnection(
        pageInfo=PageInfoApi(total_count=count, offset=offset, limit=limit),
        content=response.content.decode('utf-8'),
    )


def resolve_debug_mountings(parent, info, **kwargs):
    count, offset, limit, result = resolve_mountings_common(parent, info, False, **kwargs)

    return MountingFieldConnection(
        pageInfo=PageInfoApi(total_count=count, offset=offset, limit=limit),
        content=[
            MountingField(
                design=MountingDesignField(**item.pop('design')),
                crew=MountingCrewField(**item.pop('crew')),
                project=MountingProjectField(**item.pop('project')),
                constructionSideInfo=ConstructionSideInfo(**item.pop('constructionSideInfo')),
                **item,
            )
            for item in result
        ],
    )
