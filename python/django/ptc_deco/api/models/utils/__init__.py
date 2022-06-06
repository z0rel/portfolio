
from enum import Enum

from .get_construction_side_code import get_construction_side_code, get_construction_code
from .journal import journal_save_delete, journal_save_delete_named_meta
from .named_meta import named_meta

IMAGE_URL_MAX_LENGTH = 512
DECIMAL_PRICE_PLACES = 2


class EnumMountingRange(Enum):
    HIGEST = 100
    HIGH = 10
    MEDIUM = 0
    LOW = -10
    LOWEST = -100
