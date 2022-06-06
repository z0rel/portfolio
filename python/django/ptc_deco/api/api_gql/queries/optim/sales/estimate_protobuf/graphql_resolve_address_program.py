from graphene.relay import Node

from .graphql_resolve_estimate import ESTIMATE_SUMMARY_IGNORE_FIELDS
from ....utils.protobuf_utils import set_arg_conv, base64_bytes, PageInfoApi
from ......estimate import EstimateCalc

from ....proto import estimate_pb2
from ....utils import (
    set_arg,
    ContentFieldConnection,
    transform_kwargs_by_mapped_spec,
)
from ..estimate.db_query import get_estimate_obj_by_filter_args
from .graphql_query_field import MAPPED_KWARGS_ESTIMATE_PROTO


def tr_isoformat(v):
    return v.isoformat()


ADDRESS_PROGRAM_MAPPER = {
    'id': (lambda v: Node.to_global_id('VAddressProgrammItogsNode', v)),
    'city_id': (lambda v: Node.to_global_id('VCityOptimizedNode', v)),
    'format_id': (lambda v: Node.to_global_id('VFormatOptimizedNode', v)),
    'date_from': tr_isoformat,
    'date_to': tr_isoformat,
}


MAPPED_APPENDIX_FIELDS = [
    ('code', 'code'),
    ('signatory_one', 'signatory_one'),
    ('signatory_position', 'signatory_position'),
    ('signatory_two', 'signatory_two'),
    ('contract__code', 'contract_code'),
    ('contract__serial_number', 'contract_serial_number'),
    ('project__code', 'project_code'),
    ('project__client__bik', 'project_client_bik'),
    ('project__client__bin_number', 'project_client_bin_number'),
]


MAPPED_APPENDIX_FIELDS_CONV = [
    ('id', 'id', lambda v: Node.to_global_id('VAppendixOptimizedNode', v)),
    ('created_date', 'created_date', tr_isoformat),
    ('period_start_date', 'period_start_date', tr_isoformat),
    ('period_end_date', 'period_end_date', tr_isoformat),
    ('contract__registration_date', 'contract_registration_date', tr_isoformat),
    ('payment_date', 'payment_date', tr_isoformat),
    ('contract__payment_date', 'contract_payment_date', tr_isoformat),
    ('project__id', 'project_id', lambda v: Node.to_global_id('VProjectOptimizedNode', v)),
]


def resolve_address_programm(parent, info, **kwargs):
    transform_kwargs_by_mapped_spec(MAPPED_KWARGS_ESTIMATE_PROTO, kwargs, kwargs)

    if not kwargs:
        return ContentFieldConnection(content=None, pageInfo=PageInfoApi(total_count=0, offset=0, limit=100))

    constructor_args, calc_args = get_estimate_obj_by_filter_args(**kwargs)
    e: EstimateCalc = EstimateCalc(**constructor_args)
    e.full_calc(**calc_args)
    e.calculate_address_program()

    result_address_programm = estimate_pb2.AddressProgramm()

    e.assign_to_dst_slots_tr(
        e.itog,
        result_address_programm.itogs,
        lambda k, x: x if x is None else str(x),
        ESTIMATE_SUMMARY_IGNORE_FIELDS
    )

    appendix = result_address_programm.appendix
    for dst, src in MAPPED_APPENDIX_FIELDS:
        set_arg(appendix, dst, getattr(e.appendix_obj, src))
    for dst, src, conv in MAPPED_APPENDIX_FIELDS_CONV:
        set_arg_conv(appendix, dst, getattr(e.appendix_obj, src), conv)

    def mapper_address_programm(k, x):
        mapper = ADDRESS_PROGRAM_MAPPER.get(k)
        return mapper(x) if mapper else str(x)

    for item in e.address_programm:
        address_programm_item = result_address_programm.items.add()
        e.assign_to_dst_slots_tr(item, address_programm_item, mapper_address_programm, {'count'})
        set_arg(address_programm_item, 'count', item.count)

    return ContentFieldConnection(
        content=base64_bytes(result_address_programm.SerializeToString()),
        pageInfo=PageInfoApi(total_count=1, offset=0, limit=100),
    )
