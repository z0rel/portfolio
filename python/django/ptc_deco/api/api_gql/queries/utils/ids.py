from graphene.relay import Node


def convert_ID_to_id(id_value):
    if id_value is None:
        return None
    _type, _id = Node.from_global_id(id_value)
    return _id


def unpack_ids(kwargs, IDS_TO_UNPACK):
    for k in kwargs:
        if k in IDS_TO_UNPACK:
            kwargs[k] = convert_ID_to_id(kwargs[k])

