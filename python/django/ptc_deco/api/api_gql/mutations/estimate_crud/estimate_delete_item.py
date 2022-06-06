import graphene
from graphene.relay import Node
from graphql import GraphQLError
from typing import Optional

from .....api import models as m
from .estimate_utils import err_msg, assert_attahcment_id
from ...utils.auth.decorators import login_or_permissions_required


class DeleteEstimateItem(graphene.Mutation):
    class Arguments:
        id = graphene.ID(required=True, description='Идентификатор удаляемого элемента сметы')
        appendix_id = graphene.ID(description='Идентификатор приложения')

    ok = graphene.Boolean()

    @login_or_permissions_required(
        login_required=True,
        permissions=('api.delete_estimate_item', )
    )
    def mutate(root, info, **input):
        appendix_id = input.get('appendix_id', None)
        appendix: Optional[m.Appendix]

        if appendix_id:
            _type, appendix_id = Node.from_global_id(input['appendix_id'])
            assert_attahcment_id(_type)
            appendix = m.Appendix.objects.get(id=appendix_id)
        else:
            appendix = None

        global_id = input.pop('id')
        _type, _id = Node.from_global_id(global_id)

        if appendix:
            if _type == 'VReservationCalculatedNode':
                appendix.reservations.remove(m.Reservation.objects.get(id=_id))
            elif _type == 'VEstimateReservationsNonRtsNode':
                appendix.reservations.remove(m.Reservation.objects.get(id=_id))
            elif _type == 'VEstimateNoSidesNonRtsNode':
                appendix.additional_costs_nonrts.remove(m.EstimateNonRts.objects.get(id=_id))
            elif _type == 'VEstimateAdditionalCostsRtsNode':
                appendix.additional_costs.remove(m.AdditionalCosts.objects.get(id=_id))
            else:
                raise GraphQLError(err_msg(global_id, _type))
        else:
            if _type == 'VEstimateReservationsNonRtsNode' or _type == 'VReservationCalculatedNode':
                reservation: m.Reservation = m.Reservation.objects.filter(id=_id)
                if reservation:
                    reservation = reservation[0]
                    if reservation.estimate_non_rts_id:
                        reservation.estimate_non_rts.delete()
                    if reservation.agency_commission:
                        reservation.agency_commission.delete()
                    reservation.delete()
            elif _type == 'VEstimateNoSidesNonRtsNode':
                estimate_non_rts: m.EstimateNonRts = m.EstimateNonRts.objects.filter(id=_id)
                if estimate_non_rts:
                    estimate_non_rts = estimate_non_rts[0]
                    if estimate_non_rts.agency_commission:
                        estimate_non_rts.agency_commission.delete()
                    estimate_non_rts.delete()
            elif _type == 'VEstimateAdditionalCostsRtsNode':
                additional_costs: m.AdditionalCosts = m.AdditionalCosts.objects.filter(id=_id)
                if additional_costs:
                    additional_costs = additional_costs[0]
                    if additional_costs.agency_commission:
                        additional_costs.agency_commission.delete()
                    additional_costs.delete()
            else:
                raise GraphQLError(err_msg(global_id, _type))

        return DeleteEstimateItem(ok=True)
