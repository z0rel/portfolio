from dataclasses import dataclass
from typing import Optional

from ..utils import get_family_objects

from ptc_deco.api import models as m


class Founded(Exception):
    pass


@dataclass
class FamilyInfo:
    family: Optional[m.FamilyConstruction]
    underfamily: Optional[m.UnderFamilyConstruction]
    model: Optional[m.ModelConstruction]
    format: Optional[m.Format]
    side: Optional[m.Side]
    adv_side: Optional[m.AdvertisingSide]


def find_families_tree(families_tree, s_adv_side, s_side, s_format, s_family, get_or_create) -> FamilyInfo:
    s_underfamily = ''
    s_model = ''
    try:
        underfamilies = families_tree[s_family]
        for underfamily, models in underfamilies.items():
            s_underfamily = underfamily
            for model, formats in models.items():
                s_model = model
                for (format_value, format_code), sides in formats.items():
                    if format_value == s_format:
                        for side, (adv_sides, size, side_code) in sides.items():
                            for (adv_side, adv_side_code) in adv_sides:
                                if s_adv_side == adv_side:
                                    s_side = side
                                    raise Founded()
    except KeyError:
        s_underfamily = ''
        s_model = ''
        pass
    except Founded:
        pass

    family, underfamily, model, _format, side, adv_side = get_family_objects(
        get_or_create, m, s_family, s_underfamily, s_model, s_format, s_side, None, s_adv_side
    )

    return FamilyInfo(family=family, underfamily=underfamily, model=model, format=_format, side=side, adv_side=adv_side)
