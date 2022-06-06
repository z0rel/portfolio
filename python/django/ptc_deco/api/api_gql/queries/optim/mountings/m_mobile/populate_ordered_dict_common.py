from collections import OrderedDict
from graphene import Node
from .......api import models as m
from .......api.models.utils import EnumMountingRange


def populate_mounting_part(v, to_json):
    return [
        ('mountingRange', v.mounting_range if not to_json else EnumMountingRange(v.mounting_range).name),
        ('archived', v.archived),
        ('startMounting', v.start_mounting),
        ('endMounting', v.end_mounting),
        ('mountingDone', bool(v.mounting_done)),
        ('unmountingDone', bool(v.unmounting_done)),
        ('photos', [OrderedDict([
            ('photo', str(p.photo) if p.photo is not None else None),
            ('photoDate', p.date),
            ('photoNumber', p.num)
        ]) for p in v.photos.all()]),
        ('downloadedEarly', v.downloaded_early),
        ('comment', v.comment),
        ('design', OrderedDict([
            ('id', Node.to_global_id('VDesignOptimizedNode', str(v.design_id)) if v.design_id is not None else None),
            ('img', str(v.design_img) if v.design_img is not None else None),
            ('startedAt', v.design_started_at),
            ('title', v.design_title),
        ])),
        ('unmountingDesign', OrderedDict([
            ('id', Node.to_global_id('VDesignOptimizedNode', str(v.unmounting_design_id)) if v.unmounting_design_id is not None else None),
            ('img', str(v.unmounting_design_img) if v.unmounting_design_img is not None else None),
            ('startedAt', v.unmounting_design_started_at),
            ('title', v.unmounting_design_title),
        ])),
        ('crew', OrderedDict([
            # ('id', v.crew_id),
            ('num', v.crew_num),
            ('name', v.crew_name),
            ('phone', v.crew_phone),
            ('city', v.crew_city),
        ])),
    ]


def populate_construction_side_part(v, code):
    return [
        ('constructionSideInfo', OrderedDict([
            ('constructionId', Node.to_global_id('VConstructionOptimizedNode', v.selected_construction_id)),
            ('code', code),
            ('family', v.family_title),
            ('underfamily', v.underfamily_title),
            ('model', v.model_title),
            ('format', v.format_title),
            ('side', v.side_title),
            ('advertisingSide', v.advertising_side_title),
            ('isNonRts', v.nonrts_owner is not None),
            ('ownerTitle', v.nonrts_owner),
            ('size', v.side_size),
            ('sideCode', v.side_code),
            ('advertisingSideCode', v.advertising_side_code),
            ('address', v.address_marketing or v.address_legal),
            ('postcode', v.postcode_title),
            ('district', v.district_title),
            ('city', v.city_title),
            ('statusConnection', bool(v.status_connection)),
        ]))
    ]


def populate_ordered_dict_common(v, task_name, to_json, code):
    appendices_codes = (
            ', '.join([a.code for a in (v.reservation.appendix.all() if v.reservation else []) if a and a.code]) or None
    )
    return OrderedDict([
        ('id', Node.to_global_id('VMountingOptimizedNode', v.id)),
        *populate_mounting_part(v, to_json),
        *populate_construction_side_part(v, code),
        ('commonTaskType', v.mounting_task_title),
        ('statusConnection', bool(v.status_connection)),
        ('project', OrderedDict([
            ('title', v.project_title),
            ('code', v.project_code),
            ('appendix_code', appendices_codes),
        ])),
    ])


def get_construction_side_code(v):
    return (
        m.utils.get_construction_side_code(
            v.postcode_title, v.num_in_district, v.format_code, v.side_code, v.advertising_side_code
        )
        if (v.reservation_construction_side_id is not None or v.construction_side_id is not None)
        else (m.utils.get_construction_code(v.postcode_title, v.num_in_district, v.format_code))
    )
