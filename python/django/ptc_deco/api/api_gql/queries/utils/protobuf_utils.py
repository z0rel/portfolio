from base64 import b64encode
from graphene import String, Int, ObjectType, Field


def base64_bytes(s):
    return b64encode(s).decode('ascii')


class PageInfoApi(ObjectType):
    class Meta:
        description = 'Данные пагинации'

    offset = Int(description='Начальное смещение в полной выборке')
    limit = Int(description='Длина текущей выборки')
    total_count = Int(description='Длина полной выборки')


class ContentFieldConnection(ObjectType):
    class Meta:
        description = 'BASE64-закодированное сообщение с данными и информацей о пагинации'

    content = String(description='base64-закодированное protobuf-сообщение с данными')
    pageInfo = Field(PageInfoApi, description='Данные пагинации')


class ContentJSONFieldConnection(ObjectType):
    class Meta:
        # node = ContentJSONFieldEdge1
        description = 'JSON-закодированное сообщение с данными и информацей о пагинации'

    content = String(description='json-закодированное сообщение с данными')
    pageInfo = Field(PageInfoApi, description='Данные пагинации')


def set_arg(dst, dstarg, value):
    if value is not None:
        setattr(dst, dstarg, value)


def set_arg_getattr(dst, dstarg, src):
    value = getattr(src, dstarg)
    if value is not None:
        setattr(dst, dstarg, value)


def set_arg_isoformat(dst, dstarg, value):
    if value is not None:
        setattr(dst, dstarg, value.isoformat(timespec='seconds'))


def set_arg_conv(dst, dstarg, value, fn):
    if value is not None:
        setattr(dst, dstarg, fn(value))


def set_arg_conv_getattr(dst, dstarg, src, fn):
    value = getattr(src, dstarg)
    if value is not None:
        setattr(dst, dstarg, fn(value))

