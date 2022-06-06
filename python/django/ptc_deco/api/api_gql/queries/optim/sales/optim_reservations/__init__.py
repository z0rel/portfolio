from .......api import models as m
from ....proto import advertising_side_pb2
from ....utils import base64_bytes, ContentFieldConnection, PageInfoApi

from .db_query import db_query
from .protobuf_set import set_reservation_fields, set_construction_side_fields
from .graphql_query_field import advertising_sides_field


def resolve_advertising_sides(parent, info, **kwargs):
    construction_sides, offset, limit, count = db_query(kwargs)

    if count is None:
        count = construction_sides.count()

    construction_sides_obj = advertising_side_pb2.ConstructionSides()
    s: m.ConstructionSide

    for s in construction_sides[offset: offset + limit]:
        construction_side = construction_sides_obj.construction_side.add()

        for r in s.reservation.all():
            reservation = construction_side.reservations.add()
            set_reservation_fields(reservation, r)

        set_construction_side_fields(construction_side, s)

    result = ContentFieldConnection(
        content=base64_bytes(construction_sides_obj.SerializeToString()),
        pageInfo=PageInfoApi(total_count=count, offset=offset, limit=limit),
    )
    return result
