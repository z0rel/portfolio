from decimal import Decimal
from ptc_deco.api.models import AgencyCommission





def _get_distributed_ak(ak_obj: AgencyCommission, summary_value: Decimal) -> Decimal:
    """
        Вычислить стоимость агентской комиссии по заданному объекту
    @param ak_obj:
    @param summary_value:
    @return:
    """
    if ak_obj.value:
        return Decimal(ak_obj.value)
    elif ak_obj.percent:
        return summary_value * (Decimal(1.0) - (ak_obj.percent / Decimal(100.0)))
    else:
        return Decimal(0)


def sub(a, b):
    if a is not None:
        if b is not None:
            return a - b
        return a
    if b is not None:
        return -b

    return None


def sub_discount(a, b):
    if a is not None:
        if b is not None:
            res = a - b
            if res < 0:
                return None
            return res
        return a

    return None


def add(a, b):
    if a is not None:
        if b is not None:
            return a + b
        return a
    return b


def add2(dst, src, attr):
    attr_dst = getattr(dst, attr)
    attr_src = getattr(src, attr)
    if attr_src is not None:
        if attr_dst is not None:
            setattr(dst, attr, attr_dst + attr_src)
        else:
            setattr(dst, attr, attr_src)
