from typing import List, Dict, Set, Optional, Union
import json
from django.core.serializers.json import DjangoJSONEncoder

from .entities import (
    UEstimateNonRts,
    UAppendix,
    UAgencyCommission,
    UPartnerFk,
    UConstructionSide,
    UConstruction,
    UCityFk,
    UserFk,
    UBrand,
    UWorkingSector,
)

from ptc_deco.api import models as m


def dump_django_date(val):
    if val is None:
        return None
    return json.loads(json.dumps(val, cls=DjangoJSONEncoder))


def annotation_to_dict(obj, cls, exclude_fields: Set[str] = None) -> Dict:
    if exclude_fields:
        return {f: getattr(obj, f) for f in cls.__annotations__ if f not in exclude_fields}
    else:
        return {f: getattr(obj, f) for f in cls.__annotations__}


def partner_to_partner_fk(p: Optional[m.Partner]) -> Optional[UPartnerFk]:
    if p is None:
        return None

    return UPartnerFk(title=p.title, bin_number=p.bin_number)


def partner_fields_to_partner_fk(title: Optional[str], bin_number: Optional[str]) -> Optional[UPartnerFk]:
    if title is None and bin_number is None:
        return None

    return UPartnerFk(title=title, bin_number=bin_number)


def get_agency_commission(ak: m.AgencyCommission) -> Optional[UAgencyCommission]:
    return (
        UAgencyCommission(**annotation_to_dict(ak, UAgencyCommission, {'agent'}), agent=partner_to_partner_fk(ak.agent))
        if ak
        else None
    )


def get_appendices_id(appendices) -> Optional[List[int]]:
    return [app.id for app in appendices] if appendices else None


def get_estimate_nonrts(obj: m.EstimateNonRts, get_appendices=True) -> Optional[UEstimateNonRts]:
    if obj is None:
        return None
    return UEstimateNonRts(
        **annotation_to_dict(
            obj, UEstimateNonRts, {'appendix', 'agency_commission', 'city', 'start_period', 'end_period'}
        ),
        start_period=dump_django_date(obj.start_period),
        end_period=dump_django_date(obj.end_period),
        appendix=(get_appendices_id(obj.appendix.all()) if get_appendices else None),
        agency_commission=get_agency_commission(obj.agency_commission),
        city=get_city_fk(obj.city),
    )


def get_city_fk(obj: m.City) -> Optional[UCityFk]:
    return UCityFk(id=obj.id, title=obj.title) if obj is not None else None


class AnnotatedReservation:
    reservation_type_title: type(m.ReservationType.title)
    s_format: str
    s_side: str
    s_advertising_side: str

    inv_number_tech: str
    inv_number_buh: str
    s_family: str
    s_underfamily: str
    s_model: str

    is_nonrts: bool

    address_market: str
    address_market_postcode: str
    address_market_district: str
    address_market_city: str

    s_owner_title: Optional[str]
    s_owner_bin_number: Optional[str]


T_ANNOTATED_RESERVATION = Union[m.Reservation, AnnotatedReservation]


def get_construction_side_from_reservation(r: T_ANNOTATED_RESERVATION) -> Optional[UConstructionSide]:
    if r.construction_side_id is None:
        return None
    return UConstructionSide(
        construction=UConstruction(
            **annotation_to_dict(r, UConstruction, {'s_owner'}),
            s_owner=partner_fields_to_partner_fk(title=r.s_owner_title, bin_number=r.s_owner_bin_number),
        ),
        **annotation_to_dict(r, UConstructionSide, {'construction'}),
    )


def user_to_fk(obj: Optional[m.CustomUser]) -> Optional[UserFk]:
    if obj is None:
        return None
    return UserFk(**annotation_to_dict(obj, UserFk))


def get_project_appendix(obj: m.Appendix) -> UAppendix:
    return UAppendix(
        **annotation_to_dict(
            obj,
            UAppendix,
            {
                'creator',
                'sales_manager',
                'additionally_agreement',
                'created_date',
                'period_start_date',
                'period_end_date',
                'updated_at',
                'payment_date',
            },
        ),
        creator=user_to_fk(obj.creator),
        sales_manager=user_to_fk(obj.sales_manager),
        additionally_agreement=str(obj.additionally_agreement),
        created_date=dump_django_date(obj.created_date),
        period_start_date=dump_django_date(obj.period_start_date),
        period_end_date=dump_django_date(obj.period_end_date),
        updated_at=dump_django_date(obj.updated_at),
        payment_date=dump_django_date(obj.payment_date),
    )


def get_brand(obj: Optional[m.Brand]) -> Optional[UBrand]:
    if obj is None:
        return None
    return UBrand(
        **annotation_to_dict(obj, UBrand, {'working_sector', 'created_at', 'updated_at'}),
        created_at=dump_django_date(obj.created_at),
        updated_at=dump_django_date(obj.updated_at),
        working_sector=(
            UWorkingSector(
                id=obj.working_sector.id, title=obj.working_sector.title, description=obj.working_sector.description
            )
            if obj.working_sector is not None
            else None
        ),
    )
