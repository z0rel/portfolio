import graphene
from graphene import ID, String, Boolean, DateTime, Field, Int, List

from ...generate_order_by_class import graphql__order_by__offset__limit__mixin
from ....utils import ContentFieldConnection
from .order_by import ORDERING_FIELDS
from ....optim.generate_order_by_class import graphql__fastsearch__mixin


CODE_FILTER_FIELDS = {
    'construction__location__postcode__title__icontains': String(description='Почтовый код содержит подстроку'),
    'construction__num_in_district': Int(description='Номер в районе'),
    'advertising_side__side__format__code__icontains': String(description='Код формата'),
    'advertising_side__side__code': String(description='Код стороны'),
    'advertising_side__code': String(description='Код рекламной стороны'),
}


def gen_codes_type(name: str, description: str, parameters: dict):
    parameters['Meta'] = type(f'{name}.Meta', (), {'description': description})
    return type(
        f'{name}InputObjectType',
        (graphene.InputObjectType, ),
        parameters,
    )


advertising_sides_field = Field(
    ContentFieldConnection,
    description='Рекламные стороны (protobuf)',
    advertising_side__side__format__model__underfamily__family__id=ID(
        description='Идентификатор семейства конструкции'
    ),
    construction__location__postcode__district__city__id=ID(description='Идентификатор города'),
    construction__location__postcode__district__id=ID(description='Идентификатор района'),
    construction__location__postcode__title__icontains=String(description='Почтовый код содержит подстроку'),
    construction__num_in_district=Int(description='Номер в районе'),
    advertising_side__code=String(description='Код рекламной стороны'),
    advertising_side__side__code=String(description='Код стороны'),
    advertising_side__side__format__code__icontains=String(description='Код формата'),
    advertising_side__side__format__title__icontains=String(
        description='Наименование формата содержит заданную подстроку (регистронезависимо)'
    ),
    advertising_side__side__format__title=String(description='Наименование формата точно совпадает'),
    advertising_side__side__title__icontains=String(
        description='Наименование типа стороны содержит заданную подстроку (регистронезависимо)'
    ),
    advertising_side__side__title=String(description='Наименование типа стороны точно совпадает'),
    advertising_side__side__size__icontains=String(
        description='Размер стороны содержит заданную подстроку (регистронезависимо)'
    ),
    advertising_side__side__size=String(description='Размер стороны точно совпадает'),
    construction__status_connection=Boolean(description='Освещение - да либо нет (горит либо не горит)'),
    construction__nonrts_owner__title__icontains=String(
        description='Наименование владельца содержит заданную подстроку (регистронезависимо)'
    ),
    construction__nonrts_owner__id=ID(description='Идентификатор владельца'),
    reservation_type__title__iregex=String(
        description='Намиенование статуса бронирования соответствует заданному регулярному выражению'
    ),
    reservation__date_from__gte=DateTime(description='&gt;= заданной даты начала бронирования'),
    reservation__date_to__lte=DateTime(description='&lt;= заданной даты окончания бронирования'),
    construction__location__marketing_address__address__icontains=String(
        description='Маркетинговый адрес конструкции содержит заданную подстроку (регистронезависимо)'
    ),
    construction__location__marketing_address__id=ID(description='Идентификатор маркетингового адреса конструкции'),
    construction__location__legal_address__address__icontains=String(
        description='Юридический адрес конструкции содержит заданную подстроку (регистронезависимо)'
    ),
    package_id=ID(description='Идентификатор пакета'),
    codes=graphene.List(
        gen_codes_type(name='AdvertisingSideCode', description='Код', parameters=CODE_FILTER_FIELDS),
        description="Список кодов",
    ),
    **graphql__order_by__offset__limit__mixin('AdvertisingSidesOrderBy', ORDERING_FIELDS),
    **graphql__fastsearch__mixin(),
)
