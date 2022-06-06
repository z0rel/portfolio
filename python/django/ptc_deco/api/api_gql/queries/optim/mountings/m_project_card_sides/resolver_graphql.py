from collections import OrderedDict
from itertools import chain
from typing import Optional, Dict

from graphene import Node, DateTime


from .......api import models as m
from ....utils import PageInfoApi
from ..m_mobile import ConstructionSideInfo
from .graphql_query_field import MountingProjectCardFieldConnection
from .graphql_answer_main import (
    AnswerMountingProjectCardField,
    MountingProjectCardMountingTaskItem,
    MountingDesignProjectCard,
)
from .db_query import db_query, AnnotationReservationMountings
from ..m_mobile.populate_ordered_dict_common import populate_construction_side_part, populate_mounting_part


def get_design_graphql_object(
    design_id: Optional[int], design_dict: Dict[int, m.Design]
) -> Optional[MountingDesignProjectCard]:
    if design_id is None:
        return None

    obj: m.Design = design_dict[design_id]
    return MountingDesignProjectCard(
        id=Node.to_global_id('VDesignNode', design_id), img=obj.img, startedAt=obj.started_at, title=obj.title
    )


def resolve_mountings_project_card(parent, info, **kwargs):
    copied_kwargs = {**kwargs}

    offset_mounting_tasks = kwargs.get('mounting_tasks_offset', 0)
    limit_mounting_tasks = kwargs.get('mounting_tasks_limit', 500)
    offset_additional_photo_day = kwargs.get('additional_photo_day_offset', 0)
    limit_additional_photo_day = kwargs.get('additional_photo_day_limit', 500)
    offset_additional_photo_night = kwargs.get('additional_photo_night_offset', 0)
    limit_additional_photo_night = kwargs.get('additional_photo_night_limit', 500)

    query, offset, limit, count, mounting_filterspec = db_query(copied_kwargs)
    result = []
    design_ids = set(
        chain(
            [x.unmounting_design_id for x in query if x.unmounting_design_id is not None],
            [x.previous_design_id for x in query if x.previous_design_id is not None],
            [x.current_design_id for x in query if x.current_design_id is not None],
        )
    )
    designs = m.Design.objects.filter(id__in=design_ids)
    designs_dict = {x.id: x for x in designs}

    for v in query:
        code = m.utils.get_construction_side_code(
            v.postcode_title, v.num_in_district, v.format_code, v.side_code, v.advertising_side_code
        )
        mountings = []
        photo_day = []
        photo_night = []

        mounting_item: AnnotationReservationMountings
        for mounting_item in v.reservation_mountings.all():
            if mounting_item.mounting_task_id is None:
                dst = mountings
            elif mounting_item.mounting_task_title == 'Дополнительный дневной фотоотчет':
                dst = photo_day
            else:
                dst = photo_night

            dst.append(
                MountingProjectCardMountingTaskItem(
                    **OrderedDict(
                        [
                            ('id', Node.to_global_id('VMountingOptimizedNode', mounting_item.id)),
                            *populate_mounting_part(mounting_item, False),
                            ('mounting_task_title', mounting_item.mounting_task_title),
                        ]
                    )
                )
            )
        mountings_o = mountings[offset_mounting_tasks : offset_mounting_tasks + limit_mounting_tasks]
        photo_day_o = photo_day[offset_additional_photo_day : offset_additional_photo_day + limit_additional_photo_day]
        photo_night_o = photo_night[
            offset_additional_photo_night : offset_additional_photo_night + limit_additional_photo_night
        ]

        result.append(
            AnswerMountingProjectCardField(
                id=Node.to_global_id('VReservationOptimizedNode', v.id),
                reservation__date_from=v.date_from,
                reservation__date_to=v.date_to,
                reservation__branding=v.branding,
                constructionSideInfo=ConstructionSideInfo(**(populate_construction_side_part(v, code)[0][1])),
                comments=v.comments,
                min_date_mounting=v.min_date_mounting,
                max_date_mounting=v.max_date_mounting,
                min_date_unmounting=v.min_date_unmounting,
                max_date_unmounting=v.max_date_unmounting,
                min_photo_date=v.min_photo_date,
                min_photo_additional_day_date=v.min_photo_additional_day_date,
                min_photo_additional_night_date=v.min_photo_additional_night_date,
                unmounting_design=get_design_graphql_object(v.unmounting_design_id, designs_dict),
                previous_design=get_design_graphql_object(v.previous_design_id, designs_dict),
                current_design=get_design_graphql_object(v.current_design_id, designs_dict),
                crews=v.crews,
                mounting_tasks_total=len(mountings),
                mounting_tasks=mountings_o,
                additional_photo_day_total=len(photo_day),
                additional_photo_day=photo_day_o,
                additional_photo_night_total=len(photo_night),
                additional_photo_night=photo_night_o,
            )
        )

    return MountingProjectCardFieldConnection(
        pageInfo=PageInfoApi(total_count=count, offset=offset, limit=limit), content=result
    )
