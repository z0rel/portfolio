from typing import List, Dict, Tuple, Optional

from ptc_deco.api import models as m
from .populate_rts_row_db_values import ConstructionRtsRowDbValues

from .check_and_update_construction_side_format import check_and_update_construction_side_format
from .update_construction_side import update_construction_side
from ..utils import filter_dict, get_family_objects, GetOrCreate

# id конструкции, id рекламной стороны
T_CONSTRUCTION_SIDE_BATCH_KEY = Tuple[int, Optional[int]]


def sort_batch_key(v: ConstructionRtsRowDbValues):
    return v.r.row_idx


def resolve_family_hierarchy(get_or_create, b):
    old_s_adv_side = b.r.s_adv_side
    err = False
    family, underfamily, model, _format, side, adv_side = get_family_objects(
        get_or_create,
        m,
        b.r.s_family,
        b.r.s_underfamily,
        b.r.s_model,
        b.r.s_format,
        b.r.s_side,
        b.r.s_size,
        b.r.s_adv_side,
    )
    if adv_side is None:
        print(
            f"""Error: Unexisted advertising side for hierarhy of construction with buch_number: {
            b.r.buh_invent_number}, tech_number: {b.r.tech_invent_number}, row_idx: {b.r.row_idx
            }. Hierarhy: {b.r.s_adv_side}, {b.r.s_size}, {b.r.s_side} {b.r.s_format}, {b.r.s_model}, {
            b.r.s_underfamily}, {b.r.s_family}"""
        )
        err = True
    if model is not None and _format.model.id != b.model.id:
        print(
            f"""Error: Inconsistent model for hierarchy of construction with buch_number: {
            b.r.buh_invent_number}, tech_number: {b.r.tech_invent_number}, row_idx: {b.r.row_idx
            }.\n{''
            }    Old Hierarchy: {old_s_adv_side}, {b.r.s_size}, {b.r.s_side} {b.r.s_format}, {b.r.s_model}, {
            b.r.s_underfamily}, {b.r.s_family
            }    New Hierarchy: {b.r.s_adv_side}, {b.r.s_size}, {b.r.s_side} {b.r.s_format}, {b.r.s_model}, {
            b.r.s_underfamily}, {b.r.s_family}"""
        )
        err = True
    return family, underfamily, model, _format, side, adv_side, err


def handle_tr_family_hierarchy_error(b, tr_sides_by_ids, key):
    print(
        f"""Error: construction side with hierarchy already existed: {
        b.r.s_adv_side}, {b.r.s_size}, {b.r.s_side} {b.r.s_format}, {b.r.s_model}, {
        b.r.s_underfamily}, {b.r.s_family}\n{''
        }   Old Row Idx: {tr_sides_by_ids[key][0].r.row_idx}, New row Idx: {b.r.row_idx}"""
    )


T_CONSTRUCTION_SIDE_BATCHES = Dict[
    T_CONSTRUCTION_SIDE_BATCH_KEY, List[Tuple[m.ConstructionSide, ConstructionRtsRowDbValues]]
]


def load_or_update_construction_side(
    get_or_create: GetOrCreate,
    construction: m.Construction,
    sides_list: List[ConstructionRtsRowDbValues],
    batches: T_CONSTRUCTION_SIDE_BATCHES,
    handled_construction_sides: Dict[int, bool],
    _resolve_family_hierarchy=resolve_family_hierarchy,
    _handle_tr_family_hierarchy_error=handle_tr_family_hierarchy_error,
):
    sides_by_ids: Dict[Optional[int], List[ConstructionRtsRowDbValues]] = {}
    tr_sides_by_ids = {}

    for rv in sides_list:
        key = rv.adv_side.id if rv.adv_side is not None else None
        try:
            sides_by_ids[key].append(rv)
        except KeyError:
            sides_by_ids[key] = [rv]

    for key, it_construction_batches in sides_by_ids.items():
        it_construction_batches = list(sorted(it_construction_batches, key=sort_batch_key))
        if key is None:
            print(f'Error: Empty advertising side for side group of construction')
            print(f'-- Construction {construction}')
            for b in it_construction_batches:
                print(f'-- side data {b}')
        elif len(it_construction_batches) == 1:
            tr_sides_by_ids[key] = it_construction_batches[0]
        elif len(it_construction_batches) > 1:
            b: ConstructionRtsRowDbValues
            for i, b in enumerate(it_construction_batches):
                b.r.s_adv_side = f'{b.r.s_adv_side}{i + 1}'
                family, underfamily, model, _format, side, adv_side, err = _resolve_family_hierarchy(get_or_create, b)

                if err:
                    continue

                else:
                    b.adv_side = adv_side
                    b.side = side
                    b.format = _format
                    b.model = model
                    b.underfamily = underfamily
                    b.family = family
                    b.family_construction = family
                    key = b.adv_side.id
                    if key in tr_sides_by_ids:
                        handle_tr_family_hierarchy_error(b, tr_sides_by_ids, key)
                    else:
                        tr_sides_by_ids[key] = b

    flat_list = [item for item in tr_sides_by_ids.values()]

    for rv in flat_list:
        if rv.adv_side is None:  # Стороны конструкций для которых не задана рекламная сторона - не создаются
            continue

        key = (construction.id, rv.adv_side.id)

        construction_sides = m.ConstructionSide.objects.filter(
            construction_id=construction.id, advertising_side_id=rv.adv_side.id
        )
        args_construction_side = filter_dict(
            {
                'construction': construction,
                'advertising_side': rv.adv_side,
                'purpose_side': rv.purpose_side,
                'availability_side': rv.availability_side,
            }
        )
        if construction.crews_has_special_for_sides and rv.crew is not None:
            args_construction_side['crew_id'] = rv.crew.id

        if not construction_sides:
            new_construction_side = m.ConstructionSide(**args_construction_side)
            check_and_update_construction_side_format(
                rv.r.row_idx,
                construction,
                new_construction_side,
                'on create construction side',
            )
            try:
                batches[key].append((new_construction_side, rv))
                indices = [str(x[1].r.row_idx) for x in batches[key]]
                print(
                    f"""Error: Construction side for (construction, adv_side) {key} is non unique, row_idx: {
                    ",".join(indices)
                    }"""
                )
            except KeyError:
                batches[key] = [(new_construction_side, rv)]
        else:
            for construction_side in construction_sides:
                check_and_update_construction_side_format(
                    rv.r.row_idx,
                    construction,
                    construction_side,
                    'on update construction side',
                )
                update_construction_side(
                    rv.r.row_idx,
                    handled_construction_sides,
                    construction_side,
                    construction,
                    rv,
                )
