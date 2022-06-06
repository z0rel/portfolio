from typing import List

from .parsed_constructions import ParsedConstruction
from .populate_rts_row_db_values import ConstructionRtsRowDbValues

T_LIST_OF_RTS_DB_ROW = List[ConstructionRtsRowDbValues]
T_LIST_OF_PARSED_CONSTRUCTIONS = List[ParsedConstruction]
T_LIST_HANDLED = List[int]
