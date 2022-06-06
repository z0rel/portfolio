from ptc_deco.api import models as m


def check_and_update_construction_side_format(
    row_idx, construction: m.Construction, construction_side: m.ConstructionSide, when: str
):
    if (
        (construction.model_id is not None and construction_side.advertising_side is None)
        or (construction.model_id is None and construction_side.advertising_side is not None)
        or (construction.model_id != construction_side.advertising_side.side.format.model_id)
    ):
        print('inequality format', when)
        print(' row idx', row_idx)
        print('  ', construction)
        print('  ', construction_side)
