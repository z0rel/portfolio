"""Формат преобразования столбца входящего запроса в столбец в excel файле"""

from enum import Enum
from typing import Tuple, Dict, List, Union

from graphene.utils.str_converters import to_snake_case


class ConverterType(Enum):
    SIMPLE = 0
    ISOFORMAT = 1
    LAMBDA = 2
    TOSTR = 3




def convert_mappings(mapping: List[Union[
    str,
    Tuple[Union[str, List[str]], ConverterType],
    List[str]
]]):
    """ Преобразовать спецификацию маппинга в словарь маппинга, подходящий для быстрого парсинга """
    res = {}
    for x in mapping:
        if isinstance(x, str):
            res[x] = (to_snake_case(x), ConverterType.SIMPLE)
        elif isinstance(x, tuple):
            x, convert_type = x
            if isinstance(x, str):
                res[x] = (to_snake_case(x), convert_type)
            elif isinstance(x, list):
                if len(x) == 1:
                    x_dst = x_src = x[0]
                else:
                    x_dst, x_src = x
                res[x_dst] = (x_src, convert_type)
        elif isinstance(x, list):
            if len(x) == 1:
                x_dst = x_src = x[0]
            else:
                x_dst, x_src = x
            res[x_dst] = (x_src, ConverterType.SIMPLE)

    return res


def setattr_from_mapping(dst, src, mapping: Dict[str, Tuple[str, ConverterType]]):
    """Установить значение поля в dst по ключу из src согласно спецификации mappings"""
    for (key_dst, (key_src, convert_type)) in mapping.items():
        if convert_type == ConverterType.SIMPLE:
            val = getattr(src, key_src, None)
            if val is not None:
                setattr(dst, key_dst, val)
        elif convert_type == ConverterType.ISOFORMAT:
            val = getattr(src, key_src, None)
            if val is not None:
                setattr(dst, key_dst, val.isoformat(timespec='seconds'))
        elif convert_type == ConverterType.LAMBDA:
            val = key_src(src)
            if val is not None:
                setattr(dst, key_dst, val)
        elif convert_type == ConverterType.TOSTR:
            val = getattr(src, key_src, None)
            if val is not None:
                setattr(dst, key_dst, str(val))
