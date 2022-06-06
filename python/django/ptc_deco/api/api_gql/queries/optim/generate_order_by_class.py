from collections import OrderedDict
from typing import Dict, Tuple, List, Type, cast, Any, Set, Union, Optional
import graphene
from graphene.utils.str_converters import to_snake_case, to_camel_case
from datetime import datetime
from django.utils.timezone import make_aware


OrderByList = List[str]
RelatedSet = Set[str]

ListOfNames = List[str]

AnnotationSrcDict = List[Tuple[str, Any]]
AnnotationDict = Dict[str, Any]

OrderingSpecItem = Union[
    Tuple[
        ListOfNames,  # 0 - field names
        str,  # 1 - description
        ListOfNames,  # 2 - fields for select_related (-> related_set in result)
        AnnotationSrcDict,  # 3 - annotation dict
    ],
    Tuple[List[str], str, List[str]],
    Tuple[List[str], str],
]


OrderingSpec = Dict[str, OrderingSpecItem]


GetOrderByResult = Tuple[OrderByList, RelatedSet, AnnotationDict]


def generate_desciption(ordering_spec: OrderingSpec):
    def description(self):
        if not self:
            return ''

        asc = True
        sself = str(self).split('.')[1]
        if sself[0] == '_':
            asc = False
            sself = sself[1:]
        value: OrderingSpecItem = ordering_spec.get(to_snake_case(sself), None)
        if not value:
            return ''
        return value[1] + (' (возрастание)' if asc else ' (убывание)')

    return property(description)


def generate_order_by_class(typename: str, ordering_spec: OrderingSpec) -> Type[graphene.Enum]:
    return cast(
        Type[graphene.Enum],
        type(
            typename,
            (graphene.Enum,),
            {
                'description': generate_desciption(ordering_spec),
                **{to_camel_case(k): k for k, _ in ordering_spec.items()},
                **{'_' + to_camel_case(k): '-' + k for k, _ in ordering_spec.items()},
            },
        ),
    )


def get_order_by_list(
    kwargs, ordering_spec: OrderingSpec, annotation_dict: AnnotationDict = None
) -> Tuple[OrderByList, RelatedSet, AnnotationDict]:
    order_by_spec = OrderedDict()
    related_set: RelatedSet = set()
    order_by_annotation_dict: AnnotationDict = {}
    order_by_list: OrderByList = []
    if not annotation_dict:
        annotation_dict = {}
    if 'orderBy' in kwargs:
        ordered_specs = kwargs.pop('orderBy')
        for spec in ordered_specs:
            spec = str(spec)
            asc = True
            if spec[0] == '-':
                asc = False
                spec = spec[1:]
            spec = to_snake_case(spec)
            fields: OrderingSpecItem = ordering_spec.get(spec, None)
            if fields:
                field_names: ListOfNames = fields[0]
                order_by_spec[spec] = [asc, field_names]
                if len(fields) > 2:
                    for t in fields[2]:
                        related_set.add(t)
                    if len(fields) > 3:
                        # Добавить монтажную задачу
                        annotation_src_dict: AnnotationSrcDict = fields[3]
                        for f, v in annotation_src_dict:
                            order_by_annotation_dict[f] = v
                            order_by_spec[f] = [asc, [f]]

                for f in field_names:
                    annotation_field = annotation_dict.get(f, None)
                    if annotation_field:
                        order_by_annotation_dict[f] = annotation_field

        for ordered_item_list in order_by_spec.values():
            asc = ordered_item_list[0]
            prefix = '-' if not asc else ''
            for f in ordered_item_list[1]:
                order_by_list.append(prefix + f)

    return order_by_list, related_set, order_by_annotation_dict


def get_order_by_value(kwargs, ordering_fields: dict = {}):
    ordered_field = None
    desc = False
    if 'orderBy' in kwargs:
        ordered_field = kwargs.pop('orderBy', None)
        if ordered_field and ordered_field[0] == '-':
            ordered_field = ordered_field[1:]
            desc = True
    if ordered_field and ordering_fields:
        ordered_field = ordering_fields.get(ordered_field, None)
    if ordered_field and desc:
        ordered_field = '-' + ordered_field
    return ordered_field


def graphql__order_by__offset__limit__mixin(typename, ordering_spec):
    graphql_classname = generate_order_by_class(typename, ordering_spec)
    return {
        'offset': graphene.Int(description='Начальное смещение в полной выборке'),
        'limit': graphene.Int(description='Длина текущей выборки'),
        'orderBy': graphene.List(
            graphql_classname,
            description=('Порядок сортировки. Подчеркивание в начале имени означает ' 'сортировку по убыванию'),
        ),
    }


def graphql__fastsearch__mixin():
    return {'fastSearch': graphene.String(description='Параметр быстрого поиска')}


def get_fast_search_param(kwargs) -> Tuple[Optional[str], Optional[int], Optional[datetime]]:
    if 'fastSearch' not in kwargs:
        return None, None, None

    fast_search_spec = kwargs.pop('fastSearch')
    if fast_search_spec is None:
        return None, None, None

    fast_search_spec = fast_search_spec.strip()

    if not fast_search_spec:
        return None, None, None

    if fast_search_spec[0] == "'":
        fast_search_spec = fast_search_spec[1:]
        return fast_search_spec, None, None

    fast_search_spec_int = None
    fast_search_spec_date = None

    try:
        fast_search_spec_int = int(float(fast_search_spec))
    except Exception:
        pass

    formats = [
        '%Y-%m-%d %H:%M:%S.%f',
        '%Y-%m-%d %H:%M:%S',
        '%Y-%m-%d',
        '%d.%m.%Y %H:%M:%S.%f',
        '%d.%m.%Y %H:%M:%S',
        '%d.%m.%Y',
    ]

    if fast_search_spec_int is None:
        for fmt in formats:
            try:
                fast_search_spec_date = datetime.strptime(fast_search_spec, fmt)
                fast_search_spec_date = make_aware(fast_search_spec_date)
                break
            except Exception:
                pass

    return fast_search_spec, fast_search_spec_int, fast_search_spec_date
