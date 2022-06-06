from typing import Set

from django.db import models


def set_row_str(obj: models.Model, fields_to_update: Set, row: object, attr: str):
    old_value = getattr(obj, attr)
    val = getattr(row, attr)
    if val != old_value:
        # print(f'change {attr}: {old_value} to {val}')
        setattr(obj, attr, val)
        fields_to_update.add(attr)
        return True
    return False


def set_row_str2(obj: models.Model, fields_to_update: Set, row: object, attr: str, attr2: str):
    old_value = getattr(obj, attr)
    val = getattr(row, attr2) if row is not None else None
    if val != old_value:
        # print(f'change {attr}: {old_value} to {val}')
        setattr(obj, attr, val)
        fields_to_update.add(attr)
        return True
    return False


def set_row_db_id(obj: models.Model, fields_to_update: Set, row: object, attr: str):
    old_value = getattr(obj, attr + '_id')
    val = getattr(row, attr)
    if val is not None:
        val = val.id

    if val != old_value:
        # print(f'change {attr}_id: {old_value} to {val}')
        setattr(obj, attr + '_id', val)
        fields_to_update.add(attr + '_id')
        return True
    return False
