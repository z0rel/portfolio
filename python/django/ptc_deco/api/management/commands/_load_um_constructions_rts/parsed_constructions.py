from dataclasses import dataclass

from ptc_deco.api import models as m
from .populate_rts_row_db_values import ConstructionRtsRowDbValues
from ..utils import Action


@dataclass
class ParsedConstruction:
    construction: m.Construction
    db_row: ConstructionRtsRowDbValues
    action: Action


