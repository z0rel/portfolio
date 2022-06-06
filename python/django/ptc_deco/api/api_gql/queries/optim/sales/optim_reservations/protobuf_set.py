from graphene.relay import Node

from .......api import models as m
from ....proto import advertising_side_pb2
from ....utils import set_arg_isoformat, set_arg


def set_construction_side_fields(construction_side: advertising_side_pb2.ConstructionSide, s: m.ConstructionSide):
    construction_side.id = Node.to_global_id('VConstructionSideOptimizedNode', s.id)
    construction_side.status_connection = bool(s.status_connection)
    set_arg(construction_side, 'package', s.package_title)
    set_arg(construction_side, 'district_title', s.district_title)
    set_arg(construction_side, 'owner_title', s.nonrts_owner_title)
    set_arg(construction_side, 'is_non_rts', s.is_nonrts)
    set_arg(construction_side, 'num_in_district', s.num_in_district)
    set_arg(construction_side, 'postcode_title', s.postcode_title)
    set_arg(construction_side, 'city_title', s.city_title)
    set_arg(construction_side, 'marketing_address', s.marketing_address)
    set_arg(construction_side, 'legal_address', s.legal_address)

    if s.advertising_side is not None:
        set_arg(construction_side, 'advertising_side_code', s.advertising_side_code)
        set_arg(construction_side, 'side_code', s.side_code)
        set_arg(construction_side, 'format_code', s.format_code_side)
        set_arg(construction_side, 'format_title', s.format_title_side)
        set_arg(construction_side, 'advertising_side_title', s.advertising_side_title)
        set_arg(construction_side, 'size', s.side_size)
        set_arg(construction_side, 'side_title', s.side_title)


def set_reservation_fields(reservation: advertising_side_pb2.Reservation, r):
    reservation.id = Node.to_global_id('VReservationOptimizedNode', r.id)
    reservation.reservation_type_id = Node.to_global_id('VReservationTypeOptimizedNode', r.reservation_type_id)
    reservation.project_id = Node.to_global_id('VProjectOptimizedNode', r.project_id)
    set_arg_isoformat(reservation, 'date_from', r.date_from)
    set_arg_isoformat(reservation, 'date_to', r.date_to)
    set_arg_isoformat(reservation, 'project_start_date', r.project_start_date)
    set_arg_isoformat(reservation, 'project_created_at', r.project_created_at)
    set_arg(reservation, 'reservation_type_title', r.reservation_type_title)
    set_arg(reservation, 'project_comment', r.project_comment)
    set_arg(reservation, 'project_num_in_year', r.project_num_in_year)
    set_arg(reservation, 'project_code', r.project_code)
    set_arg(reservation, 'project_title', r.project_title)
    set_arg(reservation, 'sales_manager_first_name', r.project_sales_manager_first_name)
    set_arg(reservation, 'sales_manager_last_name', r.project_sales_manager_last_name)
    set_arg(
        reservation,
        'back_office_manager_first_name',
        r.project_back_office_manager_first_name,
    )
    set_arg(
        reservation,
        'back_office_manager_last_name',
        r.project_back_office_manager_last_name,
    )
    set_arg(reservation, 'brand_title', r.project_brand_title)
    set_arg(reservation, 'branding', r.branding)
