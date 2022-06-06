import graphene
from graphene import Int, String, DateTime, Float, ObjectType, Boolean, ID, Field, List, Argument, InputObjectType
from graphene.relay import Node
from graphql import GraphQLError

from .....api import models as m
from .estimate_utils import err_msg, assert_project_id, assert_attahcment_id, page_item_err_msg
from ...utils.auth.decorators import login_or_permissions_required


class EstimateNonrtsArgument(InputObjectType):
    class Meta:
        description = 'Параметр редактирования элемента сметы НОН РТС'

    city = ID(description='Город')
    title = String(description='Наименоваие услуги')
    count = Int(description='Количество')
    start_period = DateTime(description='Дата начала периода')
    end_period = DateTime(description='Дата окончания периода')
    # TODO: у EstimateNonRTS - startPeriod, EndPeriod

    incoming_rent = Float(description='Стоимость покупки аренды')
    incoming_tax = Float(description='Стоимость покупки налогов')
    incoming_printing = Float(description='Стоимость покупки печати')
    incoming_manufacturing = Float(description='Стоимость покупки производства')
    incoming_installation = Float(description='Стоимость покупки монтажа')
    incoming_additional = Float(description='Стоимость покупки доп. расходов')
    sale_rent = Float(description='Стоимость продажи аренды')
    sale_tax = Float(description='Стоимость продажи налогов')
    sale_printing = Float(description='Стоимость продажи печати')
    sale_manufacturing = Float(description='Стоимость продажи производства')
    sale_installation = Float(description='Стоимость продажи монтажа')
    sale_additional = Float(description='Стоимость продажи доп. расхода')
    agency_commission_value = Float(description='Значение агентской комиссии')
    agency_commission_percent = Float(description='Процент агентской комиссии')
    branding = Boolean(description='Необходим брендинг')

    project = ID(description='not used')
    appendix = ID(description='not used')


class EstimateAgencyComission(InputObjectType):
    class Meta:
        description = 'Параметр редактирования агентской комиссии в смете'

    value = Float(description='Значение агентской комиссии')
    percent = Float(description='Процент агентской комиссии')
    to_nalog = Boolean(description='Распространяется на налог')
    to_mount = Boolean(description='Распространяется на монтаж')
    to_print = Boolean(description='Распространяется на печать')
    to_rent = Boolean(description='Распространяется на аренду')
    to_additional = Boolean(description='Распространяется на доп. расходы')


class EstimateRtsAdditionalArgument(InputObjectType):
    class Meta:
        description = 'Параметр редактирования элемента сметы Доп. расходы РТС'

    title = String(description='Наименоваие услуги')
    count = Int(description='Количество')
    discount_percent = Float(description='Процент скидки')
    price = Float(description='Стоимость')
    start_period = DateTime(description='Дата начала периода')
    end_period = DateTime(description='Дата окончания периода')
    city = ID(description='Идентификатор города')

    project = ID(description='not used')
    appendix = ID(description='not used')

    agency_commission = Field(EstimateAgencyComission, description='Агентская комиссия')


class EstimateRtsReservation(InputObjectType):
    class Meta:
        description = 'Параметр редактирования элемента сметы Бронирования РТС'

    date_from = DateTime(description='Дата начала периода')
    date_to = DateTime(description='Дата окончания периода')
    branding = Boolean(description='Необходим брендинг')
    rent_to_client_setted = Float(description='Аренда на клиента')
    discount_to_client_percent_setted = Float(description='Скидка на клиента')
    rent_to_client_after_discount_setted = Float(description='Аренда на клиента со скидкой')
    mounting_setted = Float(description='Монтаж')
    printing_setted = Float(description='Печать')
    additional_setted = Float(description='Доп. расходы.')
    nalog_setted = Float(description='Налог')
    discount_nalog_percent_setted = Float(description='Скидка на налог')
    nalog_after_discount_setted = Float(description='Налог после скидки')

    agency_commission = Field(EstimateAgencyComission, description='Агентская комиссия')


class EditNonRtsItems(graphene.Mutation):
    class Arguments:
        ids = List(ID, description='Идентификаторы элементов сметы')
        non_rts = Argument(EstimateNonrtsArgument, description='Параметр редактирования элементов сметы НОН РТС')

        rts_additional = Argument(
            EstimateRtsAdditionalArgument, description='Параметр редактирования элементов сметы РТС - Доп. расходы'
        )

        rts_reservations = Argument(
            EstimateRtsReservation, description='Параметр редактирования элементов сметы РТС - Бронирования РТС'
        )

        package_editing = Boolean(description='Выполняется ли пакетное редактирование')
        is_all = Boolean(description='Изменить все строки')
        project_id = ID(description='Идентификатор проекта')
        is_package = Boolean(description='Пакетное изменени')

    ok = Boolean()
    
    @login_or_permissions_required(login_required=True, permissions=('api.edit_estimate_item', ))
    def mutate(root, info, **input):
        ids = input['ids']
        arg_non_rts = input.get('non_rts', None)
        arg_rts_additional = input.get('rts_additional', None)
        arg_rts_reservations = input.get('rts_reservations', None)
        is_package = input.get('is_package', False)
        print(is_package)
        is_all = input.get('is_all', False)
        if not is_all:
            for item in ids:
                _type, _id = Node.from_global_id(item)
                if _type == 'VReservationCalculatedNode':
                    if arg_rts_reservations is None:
                        raise GraphQLError(f'Аргумент rtsReservations должен быть задан для id {item}')

                    reservation: m.Reservation = m.Reservation.objects.filter(id=_id).select_related(
                        'agency_commission'
                    )
                    if reservation:
                        reservation = reservation[0]
                        update_rts_reservation(reservation, arg_rts_reservations, is_package)

                elif _type == 'VEstimateReservationsNonRtsNode':
                    if arg_non_rts is None:
                        raise GraphQLError(f'Аргумент nonRts должен быть задан для id {item}')

                    reservation: m.Reservation = m.Reservation.objects.filter(id=_id).select_related(
                        'estimate_non_rts', 'agency_commission'
                    )
                    if reservation:
                        reservation = reservation[0]
                        update_reservation_non_rts(reservation, arg_non_rts, is_package)

                elif _type == 'VEstimateNoSidesNonRtsNode':
                    if arg_non_rts is None:
                        raise GraphQLError(f'Аргумент nonRts должен быть задан для id {item}')

                    estimate_non_rts: m.EstimateNonRts = m.EstimateNonRts.objects.filter(id=_id).select_related(
                        'agency_commission'
                    )
                    if estimate_non_rts:
                        estimate_non_rts = estimate_non_rts[0]
                        update_estimate_non_rts(estimate_non_rts, arg_non_rts, is_package)

                elif _type == 'VEstimateAdditionalCostsRtsNode':
                    if arg_rts_additional is None:
                        raise GraphQLError(f'Аргумент rtsAdditional должен быть задан для id {item}')

                    rts_additional: m.AdditionalCosts = m.AdditionalCosts.objects.filter(id=_id).select_related(
                        'agency_commission'
                    )
                    if rts_additional:
                        rts_additional = rts_additional[0]
                        update_rts_additional(rts_additional, arg_rts_additional, is_package)

                else:
                    raise GraphQLError(err_msg(item, _type))
        else:
            _type, project_id = Node.from_global_id(input['project_id'])
            assert_project_id(_type)
            project = m.Project.objects.get(id=project_id)
            if arg_rts_reservations:
                qs = m.Reservation.objects.filter(
                    project=project,
                    construction_side__construction__is_nonrts=False,
                    construction_side__construction__nonrts_owner__isnull=True,
                ).select_related(
                    'construction_side', 'construction_side__construction', 'estimate_non_rts', 'agency_commission'
                )
                for r in qs:
                    update_rts_reservation(r, arg_rts_reservations, True)

            elif arg_rts_additional:
                qs = m.AdditionalCosts.objects.filter(project=project).select_related('agency_commission')
                for r in qs:
                    update_rts_additional(r, arg_rts_additional, True)

            elif arg_non_rts:
                qse = m.EstimateNonRts.objects.filter(project=project).select_related('agency_commission')
                qsr = m.Reservation.objects.filter(
                    project=project,
                    construction_side__construction__is_nonrts=True,
                    construction_side__construction__nonrts_owner__isnull=False,
                ).select_related(
                    'construction_side', 'construction_side__construction', 'estimate_non_rts', 'agency_commission'
                )
                for e in qse:
                    update_estimate_non_rts(e, arg_non_rts, True)
                for r in qsr:
                    update_reservation_non_rts(r, arg_non_rts, True)

        return EditNonRtsItems(ok=True)


def attr_need_to_set(input, is_package, f):
    return (f in input and getattr(input, f) is not None) or not is_package


def update_rts_agency_commission(obj, input, is_package):
    need_save = False
    if 'agency_commission' in input:
        if not input.agency_commission and not is_package:
            if obj.agency_commission:
                obj.agency_commission.delete()
                obj.agency_commission_id = None
                need_save = True
        else:
            upd_dict = {}
            for f in input.agency_commission._meta.fields.keys():
                if attr_need_to_set(input.agency_commission, is_package, f):
                    upd_dict[f] = getattr(input.agency_commission, f)
            if upd_dict:
                if not obj.agency_commission:
                    obj.agency_commission = m.AgencyCommission.objects.create(**upd_dict)
                    need_save = True
                else:
                    m.AgencyCommission.objects.filter(id=obj.agency_commission_id).update(**upd_dict)
    return need_save


def update_rts_reservation(reservation: m.Reservation, input: EstimateRtsReservation, is_package=False):
    need_save = False
    keys = input._meta.fields.keys()
    if not input.date_from or not input.date_to:
        keys = [k for k in keys if k != 'date_from' and k != 'date_to']

    for f in keys:
        if f != 'agency_commission' and attr_need_to_set(input, is_package, f):
            setattr(reservation, f, getattr(input, f))
            need_save = True

    need_save = update_rts_agency_commission(reservation, input, is_package) or need_save

    if need_save:
        reservation.save()


def update_city_field(obj, city_global_id):
    if not city_global_id:
        obj.city = None
    else:
        _type, city_id = Node.from_global_id(city_global_id)
        obj.city = m.City.objects.get(id=city_id)


def update_rts_additional(rts_additional: m.AdditionalCosts, input: EstimateRtsAdditionalArgument, is_package=False):
    update_city_field(rts_additional, input.city)
    mask_fields = {'agency_commission', 'city', 'project', 'appendix'}
    for f in input._meta.fields.keys():
        if f not in mask_fields and attr_need_to_set(input, is_package, f):
            setattr(rts_additional, f, getattr(input, f))

    update_rts_agency_commission(rts_additional, input, is_package)

    rts_additional.save()


MAPPED_NONRTS_RESERVATION_FIELD = [
    'count',
    'incoming_rent',
    'incoming_tax',
    'incoming_printing',
    'incoming_manufacturing',
    'incoming_installation',
    'incoming_additional',
    'sale_rent',
    'sale_tax',
    'sale_printing',
    'sale_manufacturing',
    'sale_installation',
    'sale_additional',
]

MAPPED_NONRTS_FIELD_NORESERVATION_FIELD = [*MAPPED_NONRTS_RESERVATION_FIELD, 'title', 'start_period', 'end_period']


def update_estimate_non_rts(estimate_non_rts: m.EstimateNonRts, input: EstimateNonrtsArgument, is_package=False):

    # выбрать только те поля, которые заданы и записать их в словарь
    estimate_non_rts_fields = {
        k: getattr(input, k)
        for k in [f for f in MAPPED_NONRTS_FIELD_NORESERVATION_FIELD if attr_need_to_set(input, is_package, f)]
    }

    update_city_field(estimate_non_rts, input.city)

    for (key, value) in estimate_non_rts_fields.items():
        setattr(estimate_non_rts, key, value)

    update_nonrts_agency_comission(estimate_non_rts, input, is_package)
    estimate_non_rts.save()


def update_nonrts_agency_comission(obj, input: EstimateNonrtsArgument, is_package):
    need_save = False
    if 'agency_commission_percent' in input or 'agency_commission_value' in input:
        if input.agency_commission_percent is not None or input.agency_commission_value is not None:
            ak = obj.agency_commission
            upd_dict = {'to_nonrts': True}
            if 'agency_commission_value' in input:
                upd_dict['value'] = input.agency_commission_value
            if 'agency_commission_percent' in input:
                upd_dict['percent'] = input.agency_commission_percent
            if not ak:
                obj.agency_commission = m.AgencyCommission.objects.create(**upd_dict)
                need_save = True
            else:
                m.AgencyCommission.objects.filter(id=ak.id).update(**upd_dict)
        else:
            if obj.agency_commission and not is_package:
                obj.agency_commission.delete()
                obj.agency_commission_id = None
                need_save = True
    return need_save


def update_reservation_non_rts(reservation: m.Reservation, input: EstimateNonrtsArgument, is_package=False):

    # выбрать только те поля, которые заданы и записать их в словарь
    estimate_non_rts_fields = {
        k: getattr(input, k)
        for k in [f for f in MAPPED_NONRTS_RESERVATION_FIELD if attr_need_to_set(input, is_package, f)]
    }

    save_reservation = False
    save_nonrts = False

    nonrts_part = reservation.estimate_non_rts
    if estimate_non_rts_fields:
        if not nonrts_part:
            nonrts_part = m.EstimateNonRts.objects.create(**estimate_non_rts_fields)
            reservation.estimate_non_rts = nonrts_part
            save_reservation = True

        for (key, value) in estimate_non_rts_fields.items():
            setattr(nonrts_part, key, value)
            save_nonrts = True

    reservation_fields = {'start_period': 'date_to', 'end_period': 'date_from', 'branding': 'branding'}
    if not input.start_period and not input.end_period:
        reservation_fields = {'branding': 'branding'}

    for f, f_to in reservation_fields.items():
        if f in input:
            setattr(reservation, f_to, getattr(input, f))
            save_reservation = True

    save_reservation = update_nonrts_agency_comission(reservation, input, is_package) or save_reservation

    if save_nonrts:
        nonrts_part.save()
    if save_reservation:
        reservation.save()

    return EditNonRtsItems(ok=True)
