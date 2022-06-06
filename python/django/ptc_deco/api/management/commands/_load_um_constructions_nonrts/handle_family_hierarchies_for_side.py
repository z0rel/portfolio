from .populate_nonrts_row_db_values import ConstructionNonRtsDbValues
from .find_families_tree import FamilyInfo, find_families_tree


def create_family_resolver(families_tree):
    def resolve_family_hierarchy(get_or_create, b: ConstructionNonRtsDbValues):
        s_side = None
        if b.r.s_adv_side:
            s_side = b.r.s_adv_side.split(' ')[0]

        founded_families: FamilyInfo = find_families_tree(
            families_tree, b.r.s_adv_side, s_side, b.r.s_format, b.r.s_family, get_or_create
        )
        family = founded_families.family
        underfamily = founded_families.underfamily
        model = founded_families.model
        _format = founded_families.format
        side = founded_families.side
        adv_side = founded_families.adv_side

        old_s_adv_side = b.r.s_adv_side
        err = False

        if adv_side is None:
            print(
                f"""Error: Unexisted advertising side for hierarhy of construction with row_idx: {
                b.r.row_idx}. Hierarchy: {b.r.s_adv_side}, {s_side} {b.r.s_format}, {b.r.s_family}"""
            )
            err = True

        if model is not None and _format.model.id != b.format.model.id:
            print(
                f"""Error: Inconsistent model for hierarchy of construction with  row_idx: {b.r.row_idx
                }.\n{''
                }    Old Hierarchy: {old_s_adv_side}, {b.r.s_format}, {b.r.s_family
                }    New Hierarchy: {b.r.s_adv_side}, {b.r.s_format}, {b.r.s_family}"""
            )
            err = True
        return family, underfamily, model, _format, side, adv_side, err

    return resolve_family_hierarchy


def handle_tr_family_hierarchy_error(b: ConstructionNonRtsDbValues, tr_sides_by_ids, key):
    print(
        f"""Error: construction side with hierarchy already existed: {
            b.r.s_adv_side}, {b.r.s_format}, {b.r.s_family}\n Old Row Idx: {tr_sides_by_ids[key][0].r.row_idx
        }, New row Idx: {b.r.row_idx}"""
    )
