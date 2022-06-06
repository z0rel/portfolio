import graphene
from graphene import ObjectType, String, DateTime, Boolean, ID
from .......api.models.utils import EnumMountingRange


class MountingDesignField(ObjectType):
    class Meta:
        description = 'Дизайн, заданный для монтажной задачи'

    id = ID(description='Идентификатор дизайна')
    img = String(description='URL-адрес изображения дизайна монтажа')
    startedAt = DateTime(description='Дата начала действия дизайна')
    title = String(description='Название дизайна')


class MountingCrewField(ObjectType):
    class Meta:
        description = 'Экипаж, заданный для монтажной задачи'

    num = String(description='Номер экипажа')
    name = String(description='Имя монтажника')
    phone = String(description='Номер телефона монтажника')
    city = String(description='Город экипажа')


class MountingProjectField(ObjectType):
    class Meta:
        description = 'Проект и приложение, для которых создана монтажная задача'

    title = String(description='Название проекта')
    code = String(description='Код проекта')
    appendix_code = String(description='Код приложения к проекту')


class ConstructionSideInfo(ObjectType):
    class Meta:
        description = 'Данные конструкции и стороны конструкции'

    constructionId = String(description='Идентификатор конструкции')
    code = String(description='Код стороны конструкции либо код конструкции')
    family = String(description='Семейство конструкции')
    underfamily = String(description='Подсемейство конструкции')
    model = String(description='Модель конструкции')
    format = String(description='Формат конструкции')
    side = String(description='Название типа стороны конструкции')
    advertisingSide = String(description='Название типа рекламной стороны конструкции')
    size = String(description='Размер стороны конструкции')
    sideCode = String(description='Код стороны конструкции')
    advertisingSideCode = String(description='Код рекламной стороны конструкции')

    address = String(description='Адрес конструкции')
    postcode = String(description='Почтовый код конструкции')
    district = String(description='Район конструкции')
    city = String(description='Город конструкции')

    isNonRts = Boolean(description='РТС / НОН РТС')
    ownerTitle = String(description='Наименование владельца')
    statusConnection = Boolean(description='Наличие освещения')


def _optim_mounting_enum_description(v):
    if v == EnumMountingRange.HIGEST:
        return 'Внеочередной'
    elif v == EnumMountingRange.HIGH:
        return 'Высокий'
    elif v == EnumMountingRange.MEDIUM:
        return 'Средний'
    elif v == EnumMountingRange.LOW:
        return 'Низкий'
    elif v == EnumMountingRange.LOWEST:
        return 'Самый низкий'
    return ''


MOUNTING_RANGE_ENUM = graphene.Enum.from_enum(EnumMountingRange, description=_optim_mounting_enum_description)