from typing import List, Any, Dict, Set, Optional, Union
import json

from django.db.models import F, Prefetch
from tqdm import trange

from .entities import (
    UProjectList,
    UProject,
    UReservation,
    UReservationSettedPart,
    UReservationType,
    UAdditionalCosts,
    UAppendix,
    UEstimateNonRts,
)
from ptc_deco.api import models as m
from .upload_model_utils import (
    T_ANNOTATED_RESERVATION,
    annotation_to_dict,
    get_appendices_id,
    get_estimate_nonrts,
    get_agency_commission,
    get_construction_side_from_reservation,
    get_city_fk,
    get_project_appendix,
    get_brand,
    partner_to_partner_fk,
    user_to_fk, dump_django_date,
)


def get_prefetch_appendix() -> Prefetch:
    return Prefetch('appendix', queryset=m.Appendix.objects.only('id'))


def upload_model_to_dictionary() -> UProjectList:
    project_list: List[UProject] = []

    p: m.Project
    r: T_ANNOTATED_RESERVATION

    projects = m.Project.objects.prefetch_related(
        Prefetch('project_appendices', queryset=m.Appendix.objects.select_related('creator', 'sales_manager')),
        Prefetch('additional_costs_nonrts', queryset=m.EstimateNonRts.objects.prefetch_related('appendix')),
        Prefetch(
            'reservations',
            queryset=m.Reservation.objects.prefetch_related('appendix')
            .select_related('estimate_non_rts')
            .annotate(
                reservation_type_title=F('reservation_type__title'),
                s_format=F('construction_side__advertising_side__side__format__title'),
                s_side=F('construction_side__advertising_side__side__title'),
                s_advertising_side=F('construction_side__advertising_side__title'),
                inv_number_tech=F('construction_side__construction__tech_invent_number'),
                inv_number_buh=F('construction_side__construction__buh_invent_number'),
                s_family=F('construction_side__advertising_side__side__format__model__underfamily__family__title'),
                s_underfamily=F('construction_side__advertising_side__side__format__model__underfamily__title'),
                s_model=F('construction_side__advertising_side__side__format__model__title'),
                is_nonrts=F('construction_side__construction__is_nonrts'),
                location_address_market=F('construction_side__construction__location__marketing_address__address'),
                location_address_market_postcode=F(
                    'construction_side__construction__location__marketing_address__postcode__title'
                ),
                location_address_market_district=F(
                    'construction_side__construction__location__marketing_address__postcode__district__title'
                ),
                location_address_market_city=F(
                    'construction_side__construction__location__marketing_address__postcode__district__city__title'
                ),
                # address_market=F('construction_side__construction__marketing_address__address'),
                # address_market_postcode=F('construction_side__construction__marketing_address__postcode__title'),
                # address_market_district=F(
                #     'construction_side__construction__marketing_address__postcode__district__title'
                # ),
                # address_market_city=F(
                #     'construction_side__construction__marketing_address__postcode__district__city__title'
                # ),
                s_owner_title=F('construction_side__construction__nonrts_owner__title'),
                s_owner_bin_number=F('construction_side__construction__nonrts_owner__bin_number'),
            ),
        ),
        Prefetch(
            'additional_costs',
            queryset=m.AdditionalCosts.objects.prefetch_related('appendix').select_related('agency_commission', 'city'),
        ),
    ).select_related(
        'creator',
        'back_office_manager',
        'sales_manager',
        'client',
        'agency',
        'brand',
        'brand__working_sector',
        'agency_commission',
    )
    all_projects = projects.all()

    for p, _ in zip(all_projects, trange(len(all_projects), desc='projects')):
        reservations: List[UReservation] = [
            UReservation(
                id=r.id,
                base=UReservationSettedPart(**annotation_to_dict(r, UReservationSettedPart)),
                creation_date=dump_django_date(r.creation_date),
                date_from=dump_django_date(r.date_from),
                date_to=dump_django_date(r.date_to),
                branding=r.branding,
                reservation_type=UReservationType(title=r.reservation_type_title),
                appendices=get_appendices_id(r.appendix.all()),
                estimate_non_rts=get_estimate_nonrts(r.estimate_non_rts, get_appendices=False),
                agency_commission=get_agency_commission(r.agency_commission),
                construction_side=get_construction_side_from_reservation(r),
            )
            for r in p.reservations.all()
        ]

        additional_costs_nonrts: List[UEstimateNonRts] = [
            get_estimate_nonrts(x) for x in p.additional_costs_nonrts.all()
        ]

        additional_costs: List[UAdditionalCosts] = [
            UAdditionalCosts(
                **annotation_to_dict(
                    additional_cost,
                    UAdditionalCosts,
                    {'agency_commission', 'city', 'appendix', 'start_period', 'end_period'},
                ),
                start_period=dump_django_date(additional_cost.start_period),
                end_period=dump_django_date(additional_cost.end_period),
                agency_commission=get_agency_commission(additional_cost.agency_commission),
                city=get_city_fk(additional_cost.city),
                appendix=get_appendices_id(additional_cost.appendix.all()),
            )
            for additional_cost in p.additional_costs.all()
        ]

        project_appendices: Dict[int, UAppendix] = {}
        for app in p.project_appendices.all():
            app_obj = get_project_appendix(app)
            project_appendices[app_obj.id] = app_obj

        project_list.append(
            UProject(
                **annotation_to_dict(
                    p,
                    UProject,
                    {
                        'brand',
                        'client',
                        'agency',
                        'creator',
                        'back_office_manager',
                        'sales_manager',
                        'agency_commission',
                        'reservations',
                        'additional_costs_nonrts',
                        'additional_costs',
                        'project_appendices',
                        'start_date',
                        'created_at',
                        'updated_at'
                    },
                ),
                brand=get_brand(p.brand),
                client=partner_to_partner_fk(p.client),
                agency=partner_to_partner_fk(p.agency),
                creator=user_to_fk(p.creator),
                back_office_manager=user_to_fk(p.back_office_manager),
                sales_manager=user_to_fk(p.sales_manager),
                agency_commission=get_agency_commission(p.agency_commission),
                reservations=reservations,
                additional_costs_nonrts=additional_costs_nonrts,
                additional_costs=additional_costs,
                project_appendices=project_appendices,
                start_date=dump_django_date(p.start_date),
                created_at=dump_django_date(p.created_at),
                updated_at=dump_django_date(p.updated_at),
            )
        )

    return UProjectList(project_list=project_list)
