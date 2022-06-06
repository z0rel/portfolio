import copy
from graphene.relay import Node
from graphql.execution.base import ResolveInfo
from graphql.language.ast import Variable

from graphene_django import DjangoObjectType
import django_filters
from django.db.models import Q, Count
from builtins import classmethod

from .optim.generate_order_by_class import get_fast_search_param
from ..utils.auth.decorators import login_or_permissions_required


def _create_gql_node_class(
        classname,
        attrs,
        extra_fields=None,
        fast_search_fields=None,
        login_required=False,
        permissions=None,
):
    if not extra_fields:
        extra_fields = {}
    if not fast_search_fields:
        fast_search_fields = {}
    return type(classname,
                (DjangoObjectType,),
                {
                    'get_queryset': classmethod(
                        login_or_permissions_required(
                            login_required,
                            permissions
                        )(get_queryset)
                    ),
                    'Meta': type(f'{classname}.Meta', (), attrs),
                    **extra_fields,
                    **fast_search_fields,
                })


def get_queryset(cls, queryset, info: ResolveInfo):
    if info.field_asts:
        fast_search_param_name = 'fastSearch'
        args = info.field_asts[0].arguments
        fast_search = next((a for a in args if a.name.value == fast_search_param_name), False)

        if fast_search:
            if isinstance(fast_search.value, Variable):
                fast_search = info.variable_values.get(fast_search.value.name.value, False)
            else:
                fast_search = fast_search.value.value  # fasts_search.value is StringValue

        if fast_search:
            fast_str, fast_int, fast_date = get_fast_search_param({fast_search_param_name: fast_search})

            fast_search_str_fields = []
            if fast_str is not None and hasattr(cls, 'fast_search_str_fields'):
                fast_search_str_fields = cls.fast_search_str_fields
            fast_search_int_fields = []
            if fast_int is not None and hasattr(cls, 'fast_search_int_fields'):
                fast_search_int_fields = cls.fast_search_int_fields
            fast_search_date_fields = []
            if fast_date is not None and hasattr(cls, 'fast_search_date_fields'):
                fast_search_date_fields = cls.fast_search_date_fields
            fast_search_count_fields = []
            if fast_int is not None and hasattr(cls, 'fast_search_count_fields'):
                fast_search_count_fields = cls.fast_search_count_fields

            q_str_objects = Q()
            if fast_search_str_fields:
                for field_name in fast_search_str_fields:
                    q_str_objects |= Q(**{f'{field_name}__icontains': fast_str})

            q_int_objects = Q()
            if fast_search_int_fields:
                for field_name in fast_search_int_fields:
                    q_int_objects |= Q(**{field_name: fast_int})

            q_date_objects = Q()
            if fast_search_date_fields:
                for field_name in fast_search_date_fields:
                    q_date_objects &= Q(**{f'{field_name}__year': fast_date.year})
                    q_date_objects &= Q(**{f'{field_name}__month': fast_date.month})
                    q_date_objects &= Q(**{f'{field_name}__day': fast_date.day})

            q_count_objects = Q()
            if fast_search_count_fields:
                for field_name in fast_search_count_fields:
                    queryset = queryset.annotate(**{f'{field_name}_count': Count(f'{field_name}')})
                    q_count_objects |= Q(**{f'{field_name}_count': fast_int})

            queryset = queryset.filter(
                Q(q_str_objects) |
                Q(q_int_objects) |
                Q(q_date_objects) |
                Q(q_count_objects)
            ).distinct()
    return queryset


def cr_node_class(
    model_class,
    exact_fields=None,
    exact_icontains_fields=None,
    exact_lt_gt_fields=None,
    exact_icontains_regex_fields=None,
    isnull_fields=None,
    src_attrs=None,
    optim_queryset=None,
    extra_fields=None,
    skip_filterset_assertion=False,
    order_by=None,
    extra_filter_fields=None,
    fast_search_int_fields=None,
    fast_search_str_fields=None,
    fast_search_date_fields=None,
    fast_search_count_fields=None,
    login_required=False,
    permissions=None,
):
    filter_fields = {}
    if extra_filter_fields is None:
        extra_filter_fields = {}

    if exact_fields:
        for field_name in exact_fields:
            filter_fields[field_name] = ['exact', 'isnull']

    if exact_icontains_fields:
        for field_name in exact_icontains_fields:
            filter_fields[field_name] = ['exact', 'icontains']

    if exact_lt_gt_fields:
        for field_name in exact_lt_gt_fields:
            filter_fields[field_name] = ['exact', 'lt', 'gt', 'lte', 'gte']

    if exact_icontains_regex_fields:
        for field_name in exact_icontains_regex_fields:
            filter_fields[field_name] = ['exact', 'icontains', 'regex', 'iregex']

    if isnull_fields:
        for field_name in isnull_fields:
            filter_fields[field_name] = ['isnull']

    attrs_unoptim = {
        'model': model_class,
        'interfaces': (Node,),
        'description': model_class._meta.verbose_name,
    }
    if filter_fields:
        attrs_unoptim['filter_fields'] = copy.deepcopy(filter_fields)

    if src_attrs:
        for k, v in src_attrs.items():
            attrs_unoptim[k] = v

    extra_fields_args = {}

    if extra_fields:
        for (field_name, field_type, resolver) in extra_fields:
            extra_fields_args[field_name] = field_type
            extra_fields_args['resolve_' + field_name] = resolver

    fast_search_fields = {}

    if fast_search_int_fields or fast_search_str_fields or fast_search_date_fields or fast_search_count_fields:
        fast_search = True
        if fast_search_int_fields:
            fast_search_fields['fast_search_int_fields'] = fast_search_int_fields
        if fast_search_str_fields:
            fast_search_fields['fast_search_str_fields'] = fast_search_str_fields
        if fast_search_date_fields:
            fast_search_fields['fast_search_date_fields'] = fast_search_date_fields
        if fast_search_count_fields:
            fast_search_fields['fast_search_count_fields'] = fast_search_count_fields
    else:
        fast_search = False

    non_optimized_node = _create_gql_node_class(
        f'V{model_class.__name__}Node',
        attrs_unoptim,
        extra_fields_args,
        fast_search_fields,
        login_required,
        permissions,
    )
    if not optim_queryset and not skip_filterset_assertion:
        return fast_search, non_optimized_node, None, model_class

    attrs_optim_filter = {
        'model': model_class,
    }
    if filter_fields:
        attrs_optim_filter['fields'] = filter_fields

    def _qs(self):
        if not hasattr(self, '_qs'):
            qs = optim_queryset(self.queryset)
            # print(qs.explain())
            if self.is_bound:
                # ensure form validation before filtering
                self.errors
                qs = self.filter_queryset(qs)
            self._qs = qs
        return self._qs

    filter_classname = f'V{model_class.__name__}FilterSet'

    common_filter_class_attr = {
        'Meta': type(f'{filter_classname}.Meta', (), attrs_optim_filter),
        **({'order_by': django_filters.OrderingFilter(fields=tuple(order_by))} if order_by else {}),
        **extra_filter_fields
    }

    if skip_filterset_assertion:
        def filter_queryset(self, queryset):
            for name, value in self.form.cleaned_data.items():
                queryset = self.filters[name].filter(queryset, value)
            return queryset

        filter_class = type(
            filter_classname,
            (django_filters.FilterSet,),
            {
                **common_filter_class_attr,
                'filter_queryset': filter_queryset,
                **({'qs': property(_qs)} if optim_queryset else {}),
            },
        )
    else:
        filter_class = type(
            filter_classname,
            (django_filters.FilterSet,),
            {
                **common_filter_class_attr,
                'qs': property(_qs)},
        )

    attrs_optim = {
        'model': model_class,
        'interfaces': (Node,),
        'filterset_class': filter_class,
        'description': model_class._meta.verbose_name,
    }
    optimized_node = _create_gql_node_class(
        f'V{model_class.__name__}OptimizedNode',
        attrs_optim,
        extra_fields_args,
        fast_search_fields,
        login_required,
        permissions,
    )
    if skip_filterset_assertion:
        model_class.gql_container = optimized_node

    return fast_search, optimized_node, non_optimized_node if not skip_filterset_assertion else None, model_class
