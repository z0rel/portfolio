from dataclasses import dataclass
from typing import Optional, List, Dict, Any, Set, Tuple

from dataclasses_json import dataclass_json
from django.db.models import QuerySet

from .._load_partners.populate_partner_row_from_sheet import LINKED_CONTRAGENTS_REPLACEMETNS
from ..utils import set_row_str
from ptc_deco.api import models as m
from ..utils.update_db_obj_fields import set_row_str2

T_DECIMAL = type(m.agency_comission.DiscountMixinPartnerAndProject.discount_client_percent)
T_DATETIME = type(m.Project.start_date)


def check_nonunique(obj: QuerySet, key, prefix):
    if len(obj) > 1:
        print(f'ERROR: {prefix} - non unique: {obj}')
        return obj[0]
    elif len(obj) == 0:
        print(f'ERROR: {prefix} - not existed key {key}')
        return None
    return obj[0]


def check_nonunique_only(obj: QuerySet, prefix):
    if len(obj) > 1:
        print(f'ERROR: {prefix} - non unique: {obj}')
        return obj[0]
    elif len(obj) == 0:
        return None
    return obj[0]


def dataclass_to_dict(obj, exclude_fields: Set[str] = None) -> Dict[str, Any]:
    if exclude_fields:
        return {f: getattr(obj, f) for f in obj.__annotations__ if f not in exclude_fields}
    else:
        return {f: getattr(obj, f) for f in obj.__annotations__}


def check_in_cache(f):
    def wrapper(self, *args, cache):
        cc = self.cache_key()
        try:
            if cc in cache:
                return cache[cc]
        except Exception as e:
            print(cc)
            raise

        return f(self, *args, cache)

    return wrapper


@dataclass_json
@dataclass
class UPartnerFk:  # +
    title: Optional[str]
    bin_number: Optional[str]

    def cache_key(self) -> Tuple:
        return 'UPartnerFk', self.title, self.bin_number

    @check_in_cache
    def resolve(self, cache: Dict) -> Optional[m.Partner]:
        self.title = self.title.strip() if self.title else None
        title = LINKED_CONTRAGENTS_REPLACEMETNS.get(self.title, self.title)

        if title != self.title:  # в случае подстановки из старой базы - поиск без БИН, т.к. он не задавался
            obj: Optional[m.Partner] = check_nonunique(m.Partner.objects.filter(title=title), self, 'Partner')
        else:
            obj: Optional[m.Partner] = check_nonunique(
                m.Partner.objects.filter(title=title, bin_number=self.bin_number), self, 'Partner'
            )

        cache[self.cache_key()] = obj
        return obj


@dataclass_json
@dataclass
class UserFk:  # +
    id: int
    username: Optional[str]
    first_name: Optional[str]
    last_name: Optional[str]
    name: Optional[str]
    email: Optional[str]
    password: Optional[str]

    def cache_key(self) -> Tuple:
        return 'UserFk', self.username, self.first_name, self.last_name, self.name, self.email

    @check_in_cache
    def resolve(self, cache: Dict) -> Optional[m.Partner]:
        obj: Optional[m.Partner] = check_nonunique(
            m.CustomUser.objects.filter(**dataclass_to_dict(self, {'id', 'password'})), self, 'User'
        )

        cache[self.cache_key()] = obj
        return obj


@dataclass_json
@dataclass
class UDesign:
    img: Optional[str]
    is_current: bool
    started_at: T_DATETIME
    title: Optional[str]
    archived: bool


@dataclass_json
@dataclass
class UAdvertPromoCompany:
    start: T_DATETIME
    title: str
    is_archived: bool
    city: str
    project_id: Optional[int]


@dataclass_json
@dataclass
class UWorkingSector:
    id: int
    title: Optional[str]
    description: Optional[str]

    def cache_key(self) -> Tuple:
        return 'UWorkingSector', self.id, self.title, self.description

    @check_in_cache
    def resolve_and_update(self, cache: Dict) -> Optional[m.Partner]:
        as_dict = dataclass_to_dict(self, {'id', 'description'})
        obj: Optional[m.WorkingSector] = check_nonunique(
            m.WorkingSector.objects.filter(**as_dict), self, 'WorkingSector'
        )
        if obj is None:
            obj = m.WorkingSector(**as_dict)
            obj.save()
        elif obj.description != self.description:
            obj.description = self.description
            obj.save(update_fields=['description'])

        cache[self.cache_key()] = obj
        return obj


@dataclass_json
@dataclass
class UCityFk:
    id: int
    title: Optional[str]

    def cache_key(self) -> Tuple:
        return 'UCityFk', self.id, self.title

    @check_in_cache
    def resolve(self, cache: Dict) -> Optional[m.Partner]:
        obj: Optional[m.Partner] = check_nonunique(
            m.City.objects.filter(**dataclass_to_dict(self, {'id'})), self, 'City'
        )

        cache[self.cache_key()] = obj
        return obj


@dataclass_json
@dataclass
class UReservationType:  # +
    title: str

    def cache_key(self) -> Tuple:
        return 'UReservationType', self.title

    @check_in_cache
    def resolve_or_create(self, cache: Dict) -> Optional[m.ReservationType]:
        obj: Optional[m.ReservationType] = check_nonunique(
            m.ReservationType.objects.filter(**dataclass_to_dict(self)), self, 'ReservationType'
        )

        if obj is None:
            obj = m.ReservationType(title=self.title)
            obj.save()

        cache[self.cache_key()] = obj
        return obj


@dataclass_json
@dataclass
class UBrand:
    id: int
    code: Optional[str]
    title: str
    created_at: T_DATETIME
    updated_at: T_DATETIME
    working_sector: Optional[UWorkingSector]

    def cache_key(self) -> Tuple:
        return 'UBrand', self.id, self.code, self.title

    @check_in_cache
    def resolve_or_update(self, cache: Dict) -> Optional[m.Brand]:
        working_sector_obj = (
            self.working_sector.resolve_and_update(cache=cache) if self.working_sector is not None else None
        )

        as_dict = dataclass_to_dict(self, {'id', 'working_sector'})
        obj: Optional[m.Brand] = check_nonunique_only(
            m.Brand.objects.filter(**as_dict, working_sector=working_sector_obj),
            'Brand',
        )

        if obj is None:
            obj = m.Brand(**as_dict, working_sector=working_sector_obj)
            obj.save()
        else:
            fields_to_update = set()
            set_row_str(obj, fields_to_update, self, 'code')
            set_row_str(obj, fields_to_update, self, 'title')
            set_row_str(obj, fields_to_update, self, 'created_at')
            set_row_str(obj, fields_to_update, self, 'updated_at')
            set_row_str2(obj, fields_to_update, working_sector_obj, 'working_sector', 'id')
            if fields_to_update:
                obj.save(update_fields=sorted(fields_to_update))

        cache[self.cache_key()] = obj
        return obj


@dataclass_json
@dataclass
class UConstruction:  # +
    id: int
    inv_number_tech: Optional[str]
    inv_number_buh: Optional[str]

    s_family: str
    s_underfamily: str
    s_model: str

    s_owner: Optional[UPartnerFk]

    is_nonrts: bool

    location_address_market: str
    location_address_market_postcode: str
    location_address_market_district: Optional[str]
    location_address_market_city: str

    # address_market: str
    # address_market_postcode: str
    # address_market_district: str
    # address_market_city: str

    def cache_key(self) -> Tuple:
        return (
            'UConstruction',
            self.id,
            self.inv_number_tech,
            self.inv_number_buh,
            self.s_family,
            self.s_underfamily,
            self.s_model,
            self.s_owner.cache_key() if self.s_owner else None,
            self.is_nonrts,
            self.location_address_market,
            self.location_address_market_city,
        )

    @check_in_cache
    def resolve(self, cache: Dict) -> Optional[m.Construction]:
        if not self.is_nonrts:
            obj: Optional[m.Construction] = check_nonunique(
                m.Construction.objects.filter(
                    buh_invent_number=self.inv_number_buh, tech_invent_number=self.inv_number_tech
                ),
                self,
                'Construction',
            )
        else:
            owner = self.s_owner.resolve(cache=cache) if self.s_owner is not None else None
            obj: Optional[m.Construction] = check_nonunique(
                m.Construction.objects.filter(
                    model__underfamily__family__title=self.s_family,
                    model__underfamily__title=self.s_underfamily,
                    model__title=self.s_model,
                    nonrts_owner=owner,
                    marketing_address__address=self.location_address_market,
                    marketing_address__postcode__district__city__title=self.location_address_market_city,
                ),
                self,
                'Construction',
            )

        cache[self.cache_key()] = obj
        return obj


@dataclass_json
@dataclass
class UConstructionSide:  # +
    id: int
    construction: UConstruction

    s_format: str
    s_side: str
    s_advertising_side: str

    def cache_key(self) -> Tuple:
        return 'UConstructionSide', self.id, *self.construction.cache_key()

    def get_model_obj(self, cache: Dict, advside_title: str) -> Optional[m.ConstructionSide]:
        return check_nonunique_only(
            m.ConstructionSide.objects.filter(
                construction=self.construction.resolve(cache=cache),
                advertising_side__title=advside_title,
                advertising_side__side__title=self.s_side,
                advertising_side__side__format__title=self.s_format,
            ),
            'ConstructionSide',
        )

    @check_in_cache
    def resolve(self, project_obj, reservation_obj, cache: Dict) -> Optional[m.ConstructionSide]:
        obj = self.get_model_obj(cache, self.s_advertising_side)
        if obj is None:
            # в старой базе не подгружались задублированные в исходном excel файле стороны Стаичная A1 и Статичная А2,
            # они грузились как Статичная А
            obj = self.get_model_obj(cache, self.s_advertising_side + '1')
            if obj is None:
                print(
                    f"""---- ERROR: None side for project code={project_obj.code} num_in_year={project_obj.num_in_year
                }, title={project_obj.title} start={project_obj.start_date} brand={project_obj.brand} {
                reservation_obj.date_from}-{reservation_obj.date_to} {reservation_obj.reservation_type}, {self}"""
                )
        cache[self.cache_key()] = obj
        return obj


@dataclass_json
@dataclass
class UAgencyCommission:  # +
    id: int
    to_rent: bool
    to_nalog: bool
    to_print: bool
    to_mount: bool
    to_additional: bool
    to_nonrts: bool
    percent: Optional[T_DECIMAL]
    value: Optional[T_DECIMAL]
    agent: Optional[UPartnerFk]

    def cache_key(self) -> Tuple:
        return 'UAgencyCommission', self.id

    def create(self, cache: Dict) -> m.AgencyCommission:
        agent_obj = self.agent.resolve(cache) if self.agent is not None else None
        as_dict = dataclass_to_dict(self, {'id', 'agent'})
        return m.AgencyCommission(**as_dict, agent=agent_obj)


@dataclass_json
@dataclass
class UEstimateNonRts:
    id: int
    city: Optional[UCityFk]
    count: Optional[int]
    title: Optional[str]
    start_period: Optional[T_DATETIME]
    end_period: Optional[T_DATETIME]
    incoming_rent: Optional[T_DECIMAL]
    incoming_tax: Optional[T_DECIMAL]
    incoming_printing: Optional[T_DECIMAL]
    incoming_manufacturing: Optional[T_DECIMAL]
    incoming_installation: Optional[T_DECIMAL]
    incoming_additional: Optional[T_DECIMAL]
    sale_rent: Optional[T_DECIMAL]
    sale_tax: Optional[T_DECIMAL]
    sale_printing: Optional[T_DECIMAL]
    sale_manufacturing: Optional[T_DECIMAL]
    sale_installation: Optional[T_DECIMAL]
    sale_additional: Optional[T_DECIMAL]

    appendix: Optional[List[int]]
    agency_commission: Optional[UAgencyCommission]

    def cache_key(self) -> Tuple:
        return 'UEstimateNonRts', self.id

    # def create(self, cache: Dict, agency_commission: m.AgencyCommission) -> m.EstimateNonRts:
    #     city_obj = self.city.resolve(cache) if self.city is not None else None
    #     as_dict = dataclass_to_dict(self, {'id', 'city', 'agency_commission', 'appendix'})
    #     obj = m.AgencyCommission(**as_dict, agent=agent_obj)


@dataclass_json
@dataclass
class UAdditionalCosts:
    id: int
    title: str
    start_period: T_DATETIME
    end_period: T_DATETIME
    count: Optional[int]
    price: T_DECIMAL
    category: int

    agency_commission: Optional[UAgencyCommission]
    city: Optional[UCityFk]

    appendix: List[int]

    def cache_key(self) -> Tuple:
        return 'UAdditionalCosts', self.id


@dataclass_json
@dataclass
class UReservationSettedPart:  # +
    rent_by_price_setted: Optional[T_DECIMAL]
    mounting_setted: Optional[T_DECIMAL]
    printing_setted: Optional[T_DECIMAL]
    additional_setted: Optional[T_DECIMAL]
    nalog_setted: Optional[T_DECIMAL]
    discount_price_percent_setted: Optional[T_DECIMAL]
    rent_by_price_after_discount_setted: Optional[T_DECIMAL]
    rent_to_client_setted: Optional[T_DECIMAL]
    discount_to_client_percent_setted: Optional[T_DECIMAL]
    rent_to_client_after_discount_setted: Optional[T_DECIMAL]
    discount_nalog_percent_setted: Optional[T_DECIMAL]
    nalog_after_discount_setted: Optional[T_DECIMAL]


@dataclass_json
@dataclass
class UReservation:
    id: int
    base: UReservationSettedPart

    date_from: Optional[T_DATETIME]
    date_to: Optional[T_DATETIME]

    branding: bool

    creation_date: Optional[T_DATETIME]
    reservation_type: UReservationType

    appendices: Optional[List[int]]
    estimate_non_rts: Optional[UEstimateNonRts]
    agency_commission: Optional[UAgencyCommission]
    construction_side: UConstructionSide
    # distributed_to_mounting

    def cache_key(self) -> Tuple:
        return 'UReservation', self.id


@dataclass_json
@dataclass
class UAppendix:  # +
    id: int
    code: str
    num_in_month: int
    created_date: T_DATETIME
    period_start_date: T_DATETIME
    period_end_date: T_DATETIME
    return_status: bool
    additionally_agreement: str
    updated_at: T_DATETIME
    signatory_one: str
    signatory_two: str
    payment_date: T_DATETIME
    signatory_position: str
    is_archive: bool
    creator: Optional[UserFk]
    sales_manager: Optional[UserFk]

    def cache_key(self) -> Tuple:
        return 'UAppendix', self.id

    @staticmethod
    def resolve_by_id(id_value, cache: Dict) -> m.Appendix:
        return cache[('UAppendix', id_value)]


@dataclass_json
@dataclass
class UProject:  # +
    id: int
    discount_price_percent: Optional[T_DECIMAL]
    discount_client_percent: Optional[T_DECIMAL]
    discount_nalog_percent: Optional[T_DECIMAL]
    code: Optional[str]
    num_in_year: Optional[int]
    title: Optional[str]
    comment: Optional[str]
    start_date: T_DATETIME
    created_at: T_DATETIME
    updated_at: T_DATETIME

    brand: Optional[UBrand]

    client: Optional[UPartnerFk]
    agency: Optional[UPartnerFk]

    is_archive: bool

    creator: Optional[UserFk]
    back_office_manager: Optional[UserFk]
    sales_manager: Optional[UserFk]

    agency_commission: Optional[UAgencyCommission]

    reservations: List[UReservation]
    additional_costs_nonrts: List[UEstimateNonRts]
    additional_costs: List[UAdditionalCosts]
    project_appendices: Dict[int, UAppendix]

    def cache_key(self) -> Tuple:
        return 'UProject', self.id


@dataclass_json
@dataclass
class UProjectList:
    project_list: List[UProject]


# PlacementPrice, Package, StaticAdditionalCosts, Appendix
