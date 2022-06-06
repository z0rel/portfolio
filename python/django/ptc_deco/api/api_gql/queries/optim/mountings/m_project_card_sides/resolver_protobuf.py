from ....utils import (
    base64_bytes,
    ContentFieldConnection,
    PageInfoApi,
    convert_mappings,
    ConverterType,
    setattr_from_mapping,
)
from ....proto import mounting_project_card_pb2
from .......api import models as m
from .db_query import db_query


_MAPPING_RESERVATION = convert_mappings([
        'id',
        'comments',
        (['minDateMounting'], ConverterType.ISOFORMAT),
        (['minDateUnmounting'], ConverterType.ISOFORMAT),
        (['minPhotoDate'], ConverterType.ISOFORMAT),
        (['minPhotoAdditionalDayDate'], ConverterType.ISOFORMAT),
        (['minPhotoAdditionalNightDate'], ConverterType.ISOFORMAT),
    ])

_MAPPING_CONSTRUCTIONS_SIDE = convert_mappings([
        ['constructionId', 'selected_construction_id'],
        ['family', 'family_title'],
        ['underfamily', 'underfamily_title'],
        ['model', 'model_title'],
        ['format', 'format_title'],
        ['side', 'side_title'],
        ['advertisingSide', 'advertising_side_title'],
        ['isNonRts', 'nonrts_owner'],
        ['ownerTitle', 'nonrts_owner'],
        ['size', 'side_size'],
        ['sideCode', 'side_code'],
        ['advertisingSideCode', 'advertising_side_code'],
        (['address', lambda v: v.address_marketing or v.address_legal], ConverterType.LAMBDA),
        ['postcode', 'postcode_title'],
        ['district', 'district_title'],
        ['city', 'city_title'],
        'statusConnection',
    ])


_MAPPING_PHOTOS = convert_mappings([
    ('photo', ConverterType.TOSTR), (['photoDate', 'date'], ConverterType.ISOFORMAT), ('photoNumber', 'num')
])


_MAPPING_DESIGN = convert_mappings([
        ('img', ConverterType.TOSTR),
        (['startedAt', 'design_started_at'], ConverterType.ISOFORMAT),
        ('title', 'design_title'),
    ])


_MAPPING_CREW = convert_mappings([
        ['num', 'crew_num'],
        ['name', 'crew_name'],
        ['phone', 'crew_phone'],
        ['city', 'crew_city'],
    ])


_MAPPING_MOUNTING_TASK = convert_mappings([
        'id',
        'mountingTaskTitle',
        'mountingRange' 'archived',
        ('startMounting', ConverterType.ISOFORMAT),
        ('endMounting', ConverterType.ISOFORMAT),
        'mountingDone',
        'unmountingDone',
        ('downloadedEarly', ConverterType.ISOFORMAT),
        'comment',
    ])


def resolve_mounting_project_card_protobuf(parent, info, **kwargs):
    copied_kwargs = {**kwargs}

    query, offset, limit, count, mounting_filterspec = db_query(copied_kwargs)

    reservations: mounting_project_card_pb2.ProjectCardReservations = (
        mounting_project_card_pb2.ProjectCardReservations()
    )

    for v in query:
        reservation: mounting_project_card_pb2.ProjectCardReservation = reservations.reservations.add()
        setattr_from_mapping(reservation, v, _MAPPING_RESERVATION)
        setattr_from_mapping(reservation.constructionSideInfo, v, _MAPPING_CONSTRUCTIONS_SIDE)
        reservation.constructionSideInfo.code = m.utils.get_construction_side_code(
            v.postcode_title, v.num_in_district, v.format_code, v.side_code, v.advertising_side_code
        )

        if v.reservation_mountings:
            for mounting_item in v.reservation_mountings.all():
                if mounting_item.mounting_task_id is None:
                    dst = reservation.mountingTasks.add()
                elif mounting_item.mounting_task_title == 'Дополнительный дневной фотоотчет':
                    dst = reservation.additionalPhotoDay.add()
                else:
                    dst = reservation.additionalPhotoNight.add()

                setattr_from_mapping(dst, mounting_item, _MAPPING_MOUNTING_TASK)
                setattr_from_mapping(dst.mountingDesign, mounting_item, _MAPPING_DESIGN)
                setattr_from_mapping(dst.unmountingDesign, mounting_item, _MAPPING_DESIGN)
                setattr_from_mapping(dst.crew, mounting_item, _MAPPING_CREW)
                for p in mounting_item.photos.all():
                    p_obj = dst.photos.add()
                    setattr_from_mapping(p_obj, p, _MAPPING_PHOTOS)

    result = ContentFieldConnection(
        content=base64_bytes(reservations.SerializeToString()),
        pageInfo=PageInfoApi(total_count=count, offset=offset, limit=limit),
    )
    return result
