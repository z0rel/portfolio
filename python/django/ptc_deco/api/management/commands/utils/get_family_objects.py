from typing import Optional, Tuple

from ptc_deco.api import models as m

from .get_families_tree import _FORMAT_CODES


def tr_s_side_title(title):
    if title is None:
        return None, None
    title_arr = title.split(' ')
    if len(title_arr):
        title_arr[0] = title_arr[0].capitalize()
    code = None
    if len(title_arr) > 1:
        for i in range(1, len(title_arr)):
            title_arr[i] = title_arr[i].upper()
        code = ' '.join(title_arr[1:])

    return ' '.join(title_arr), code


def get_family_objects(
    get_or_create, m, s_family, s_underfamily, s_model, s_format, s_side, s_size, s_adv_side
) -> Tuple[
    Optional[m.FamilyConstruction],
    Optional[m.UnderFamilyConstruction],
    Optional[m.ModelConstruction],
    Optional[m.Format],
    Optional[m.Side],
    Optional[m.AdvertisingSide],
]:
    family = get_or_create(m.FamilyConstruction, title=s_family) if s_family else None

    underfamily = (
        get_or_create(m.UnderFamilyConstruction, title=s_underfamily, family_id=family.id)
        if s_underfamily and family
        else None
    )

    # преобразование строк вида Б1 - антивандальная в Б1 - Антивандальная
    if s_model:
        s_model = ' '.join([x.capitalize() for x in s_model.split(' ')])

    model = (
        get_or_create(m.ModelConstruction, title=s_model, underfamily_id=underfamily.id)
        if s_model and underfamily
        else None
    )

    format_code = _FORMAT_CODES[s_format.lower()] if s_format is not None else None
    format = (
        get_or_create(m.Format, title=s_format, model_id=model.id, code=format_code) if s_format and model else None
    )

    s_side, s_side_code = tr_s_side_title(s_side)
    s_adv_side, s_adv_side_code = tr_s_side_title(s_adv_side)

    # обработка случаев когда код стороны или код рекламной стороны разобрать не удалось
    if not s_side_code and s_adv_side_code:
        s_side_code = s_adv_side_code[0]
    elif not s_adv_side_code and s_side_code:
        s_adv_side_code = s_side_code

    side = (
        get_or_create(m.Side, title=s_side, size=s_size, format_id=format.id, code=s_side_code)
        if s_side and format
        else None
    )

    adv_side = (
        get_or_create(m.AdvertisingSide, title=s_adv_side, side_id=side.id, code=s_adv_side_code)
        if s_adv_side and side
        else None
    )

    return family, underfamily, model, format, side, adv_side
