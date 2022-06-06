from django.contrib.postgres.aggregates import StringAgg, ArrayAgg
from django.db.models import Prefetch, F, Q

from ...generate_order_by_class import get_order_by_list
from ....utils import convert_ID_to_id, transform_kwargs_by_mapped_spec, get_offset_limit
from .......api import models as m

from .db_mapped_fields import MAPPED_KWARGS_WORKING_SECTORS, MAPPED_PROJECTS_KWARGS, RELATED_TABLES, ANNOTATION_FIELDS, \
    ONLY_FIELDS
from .order_by import ORDERING_FIELDS
from .db_query_fastsearch import init_fast_search_query

LIMIT = 500


def get_project_filterspec_or(kwargs, prefix=''):
    project_filterspec_or = kwargs.pop('project_filterspec_or', [])
    kwspec_ids = set()
    filterspec_first = None
    for spec in project_filterspec_or:
        kwspec = {}
        project_id = spec.get('project_id', None)
        fullmatch = spec.get('fullmatch', False)
        if project_id:
            kwspec_ids.add(convert_ID_to_id(project_id))

        project_title_iregex = spec.get('project_title__iregex', None)
        if project_title_iregex:
            kwspec[f'{prefix}title__iregex'] = project_title_iregex
        project_code_iregex = spec.get('project_code__iregex', None)
        if project_code_iregex:
            kwspec[f'{prefix}code__iregex'] = project_code_iregex if not fullmatch else f'^{project_code_iregex}$'

        if kwspec:
            if filterspec_first is None:
                filterspec_first = Q(**kwspec)
            else:
                filterspec_first = filterspec_first | Q(**kwspec)

    if kwspec_ids:
        if filterspec_first is None:
            filterspec_first = Q(**{f'{prefix}id__in': list(sorted(kwspec_ids))})
        else:
            filterspec_first = filterspec_first | Q(**{f'{prefix}id__in': list(sorted(kwspec_ids))})

    return [filterspec_first] if filterspec_first is not None else []


def db_query_projects(kwargs):
    ws_filter = {}

    project_filterspec = get_project_filterspec_or(kwargs)

    transform_kwargs_by_mapped_spec(MAPPED_KWARGS_WORKING_SECTORS, kwargs, ws_filter)
    transform_kwargs_by_mapped_spec(MAPPED_PROJECTS_KWARGS, kwargs, kwargs)

    offset, limit = get_offset_limit(kwargs, LIMIT)
    order_by_list, order_related_set, order_annotations = get_order_by_list(kwargs, ORDERING_FIELDS)

    fast_search_query = init_fast_search_query(kwargs)
    filter_query = Q(
        Q(
            Q(*project_filterspec) &
            Q(**kwargs)
        ) &
        fast_search_query
    )

    projects = (
        m.Project.objects.filter(filter_query)
        .select_related(*RELATED_TABLES)
        .only(
            *ONLY_FIELDS,
            *order_related_set
        )
        .annotate(
            **ANNOTATION_FIELDS,
            cities_list=StringAgg(
                F('project_cities__city__title'),
                delimiter=',',
                distinct=True,
                ordering=F('project_cities__city__title'),
                filter=Q(project_cities__count__gt=0)
            ),
            cities_arr=ArrayAgg(
                F('project_cities__city__title'),
                delimiter=',',
                distinct=True,
                ordering=F('project_cities__city__title'),
                filter=Q(project_cities__count__gt=0)
            ),
            **order_annotations
        )
        .order_by(*order_by_list, 'id')
    )
    count = m.Project.objects.filter(filter_query).count()

    return projects, offset, limit, count
