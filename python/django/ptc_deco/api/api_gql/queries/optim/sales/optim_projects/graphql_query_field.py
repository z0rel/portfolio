from graphene import Field, DateTime, String, ID, ObjectType, List, InputObjectType, Boolean

from ...generate_order_by_class import graphql__order_by__offset__limit__mixin, graphql__fastsearch__mixin
from ....utils import ContentFieldConnection
from .order_by import ORDERING_FIELDS


class SearchCommercialProjectSpec(InputObjectType):
    class Meta:
        description = 'Спецификация поиска проекта'

    project_id = ID(description='Идентификатор проекта')
    project_title__iregex = String(description='Имя проекта содержит (регистронезависимо)')
    project_code__iregex = String(description='Код проекта содержит (регистронезависимо)')
    fullmatch = Boolean(description='Полное соответствие')


projects_field = Field(
    ContentFieldConnection,
    created_at__gte=DateTime(description='Начало проекта &gt;= заданной даты'),
    created_at__lte=DateTime(description='Начало проекта &lt;= заданной даты'),
    brand__title__icontains=String(description='Наименование бренда содержит подстроку (регистронезависимо)'),
    project_filterspec_or=List(SearchCommercialProjectSpec, description='Спецификация поиска проекта, '
                                                                        'объединенная условием ИЛИ'),
    project_id=ID(description='Идентификатор проекта'),
    brand_id=ID(description='Идентификатор бренда'),
    client_id=ID(description='Идентификатор клиента'),
    agency_id=ID(description='Идентификатор рекламного агентства'),
    working_sector_id=ID(description='Идентификатор сектора деятельности клиента'),

    back_office_manager_id=ID(description='Идентификатор проекта'),
    sales_manager_id=ID(description='Идентификатор менеджера по продажам'),
    title__icontains=String(description='Наименование проекта содержит подстроку (регистронезависимо)'),
    code__icontains=String(description='Код содержит подстроку (регистронезависимо)'),

    client__working_sectors__description__icontains=String(
        description='Сектор деятельности клиента содержит подстроку (регистронезависимо)'
    ),
    back_office_manager__first_name__icontains=String(
        description='Имя менеджера бек-оффиса содержит подстроку (регистронезависимо)'
    ),
    back_office_manager__last_name__icontains=String(
        description='Фамилия менеджера бек-оффиса содержит подстроку (регистронезависимо)'
    ),
    sales_manager__first_name__icontains=String(
        description='Имя менеджера по продажам содержит подстроку (регистронезависимо)'
    ),
    sales_manager__last_name__icontains=String(
        description='Фамилия менеджера по продажам содержит подстроку (регистронезависимо)'
    ),
    client__title__icontains=String(description='Название клиента содержит подстроку'),
    agency__title__icontains=String(description='Название рекламного агентства содержит подстроку'),

    **graphql__order_by__offset__limit__mixin('MountingCommercialProjectsOrderBy', ORDERING_FIELDS),
    **graphql__fastsearch__mixin(),
    description='Проекты (protobuf)',
)
