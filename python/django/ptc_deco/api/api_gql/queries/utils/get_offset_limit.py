def get_offset_limit(kwargs, LIMIT):
    offset = 0
    if 'offset' in kwargs:
        offset = kwargs.pop('offset')
    limit = LIMIT
    if 'limit' in kwargs:
        limit = kwargs.pop('limit')
    return offset, limit
