from collections import OrderedDict
from graphene import Field, String, ID
from ....utils import ContentFieldConnection


QUERY_FIELDS_ESTIMATE_PROTO = OrderedDict([
    ('project__code', String(description='Код проекта')),  # project__code__exact
    ('appendix__code', String(description='Код приложения')),  # appendix__code__exact
    ('project__id', ID(description='Идентификатор проекта')),  # project__id__exact
    ('appendix__code', String(description='Код приложения')),  # appendix__code__exact
    ('appendix__id', ID(description='Идентификатор приложения')),  # appendix__id__exact
])


MAPPED_KWARGS_ESTIMATE_PROTO = {
    'project__code': ['project__code__exact', False],
    'appendix__code': ['appendix__code__exact', False],
    'project__id': ['project__id__exact', True],
    'appendix__id': ['appendix__id__exact', True],
}


field_estimate = Field(
    ContentFieldConnection,
    description='Смета (protobuf)',
    **QUERY_FIELDS_ESTIMATE_PROTO,
)


field_address_programm = Field(
    ContentFieldConnection,
    description='адресная программа (protobuf)',
    **QUERY_FIELDS_ESTIMATE_PROTO,
)
