import graphene
from graphene import ID, List, Boolean, String
from graphene.relay import Node
from graphql import GraphQLError

from .....api import models as m
from .estimate_utils import err_msg, assert_project_id, assert_attahcment_id, page_item_err_msg
from ...utils.auth.decorators import login_or_permissions_required


class AddEstimateItemToAppendix(graphene.Mutation):
    class Arguments:
        ids = List(ID, description='Идентификаторы элементов')
        is_all = Boolean(description='Добавить все элементы')
        estimate_section = String(required=True, description='Раздел сметы (бронирования, доп. расходы РТС, НОНРТС)')
        appendix_id = ID(required=True, description='Идентификатор приложения')
        project_id = ID(required=True, description='Идентификатор проекта')

    ok = Boolean()
    
    @login_or_permissions_required(login_required=False, permissions=('api.add_estimate_item_to_appendix', ))
    def mutate(root, info, **input):
        _type, appendix_id = Node.from_global_id(input['appendix_id'])
        assert_attahcment_id(_type)
        _type, project_id = Node.from_global_id(input['project_id'])
        assert_project_id(_type)

        appendix = m.Appendix.objects.get(id=appendix_id)

        ids = input.get('ids', None)
        all = input.get('is_all', False)
        estimate_section = input.get('estimate_section', False)
        reservation_ids = []
        estimate_nonrts_ids = []
        additional_costs_ids = []
        if ids:
            for item in ids:
                _type, item_id = Node.from_global_id(item)
                if _type in {
                    'VReservationCalculatedNode',
                    'VEstimateReservationsNonRtsNode',
                    'VReservationOptimizedNode',
                }:
                    reservation_ids.append(item_id)
                elif _type == 'VEstimateNoSidesNonRtsNode':
                    estimate_nonrts_ids.append(item_id)
                elif _type == 'VEstimateAdditionalCostsRtsNode':
                    additional_costs_ids.append(item_id)
                else:
                    raise GraphQLError(err_msg(item, _type))

            if reservation_ids:
                appendix.reservations.add(*m.Reservation.objects.filter(id__in=reservation_ids))
            if estimate_nonrts_ids:
                appendix.additional_costs_nonrts.add(*m.EstimateNonRts.objects.filter(id__in=estimate_nonrts_ids))
            if additional_costs_ids:
                appendix.additional_costs.add(*m.AdditionalCosts.objects.filter(id__in=additional_costs_ids))

        elif estimate_section and all:
            project = m.Project.objects.get(id=project_id)
            if estimate_section == 'reservations-rts':
                appendix.reservations.add(
                    *m.Reservation.objects.filter(
                        project=project,
                        construction_side__construction__is_nonrts=False,
                        construction_side__construction__nonrts_owner__isnull=True,
                    ).select_related('construction_side', 'construction_side__construction')
                )
            elif estimate_section == 'additional-rts':
                appendix.additional_costs.add(*m.AdditionalCosts.objects.filter(project=project))
            elif estimate_section == 'non-rtc':
                appendix.additional_costs_nonrts.add(*m.EstimateNonRts.objects.filter(project=project))
                appendix.reservations.add(
                    *m.Reservation.objects.filter(
                        project=project,
                        construction_side__construction__is_nonrts=True,
                        construction_side__construction__nonrts_owner__isnull=False,
                    ).select_related('construction_side', 'construction_side__construction')
                )
            else:
                raise GraphQLError(page_item_err_msg(estimate_section))

        else:
            raise GraphQLError(
                'Должны быть заданы либо идентификаторы для обновления, либо раздел сметы и признак all=True'
            )

        return AddEstimateItemToAppendix(ok=True)
