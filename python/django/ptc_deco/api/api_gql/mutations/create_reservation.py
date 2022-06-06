from distutils.command.install_data import install_data

from django.db.models import Prefetch

import graphene
from graphene import ID, List, Boolean, DateTime
from graphql import GraphQLError
from graphene.relay import Node
from ....api import models as m
from .estimate_crud.estimate_utils import (
    assert_project_id,
    assert_attahcment_id,
    assert_construction_side_id,
    assert_reservation_type_msg,
)
from django.db.models import Q, F
from ..utils.auth.decorators import login_or_permissions_required


def check_reservation_for_side_exists(construction_side_id, period_date_to, period_date_from, reservation_id=None):
    additional_args = []
    if reservation_id:
        additional_args.append(~Q(id=reservation_id))

    return m.Reservation.objects.filter(
        date_from__lt=period_date_to,
        date_to__gte=period_date_from,
        reservation_type__level__gt=1,
        construction_side_id=construction_side_id,
        *additional_args,
    ).exists()


class CreateOrUpdateReservation(graphene.Mutation):
    class Arguments:
        ids = List(ID, description='Идентификаторы бронирований ')
        sides_ids = List(ID, description='Идентификаторы сторон конструкций')

        date_from = DateTime(required=True, description='Дата начала бронирования')
        date_to = DateTime(required=True, description='Дата окончания бронирования')
        project = ID(required=True, description='Идентификатор проекта')
        appendix = ID(description='Идентификатор приложения')
        branding = Boolean(description='Нужен ли брендинг или нет')
        reservation_type = ID(description='Тип бронирования')

    created_reservations = List(ID, description='Идентификаторы созданных броней')
    updated_reservations = List(ID, description='Идентификаторы обновленных броней')
    changed_project = ID(description='Идентификатор обновленного проекта')
    changed_appendix = ID(description='Идентификатор обновленного приложения')
    bad_reservations = List(ID, description='Идентификаторы броней, которые не удалось изменить')
    bad_sides = List(ID, description='Идентификаторы сторон для которых бронь создать не удалось')
    ok = Boolean()

    @login_or_permissions_required(login_required=False, permissions=(
            'api.add_reservation',
            'api.change_reservation',
    ))
    def mutate(root, info, **input):
        ids = input.get('ids', None)
        sides_ids = input.get('sides_ids', None)
        period_date_from = input.get('date_from', None)
        period_date_to = input.get('date_to', None)
        project = input.get('project', None)
        appendix = input.get('appendix', None)
        branding = input.get('branding', None)
        reservation_type = input.get('reservation_type', None)

        if not reservation_type:
            raise GraphQLError('Тип бронирования должен быть задан')

        _type, _id = Node.from_global_id(reservation_type)
        assert_reservation_type_msg(_type)
        reservation_type_obj = m.ReservationType.objects.filter(id=_id)
        if reservation_type_obj:
            reservation_type = reservation_type_obj[0]
        else:
            raise GraphQLError(f'Типа бронирования с идентификатором {reservation_type} не существует')

        if not sides_ids and not ids:
            raise GraphQLError(
                'Должны быть заданы конструкции для создания бронирований или бронирования для обновления'
            )
        if not period_date_from or not period_date_to:
            raise GraphQLError('Дата начала и дата окончания должны быть заданы')
        if period_date_from > period_date_to:
            raise GraphQLError('Дата начала должна быть меньше либо равна даты окончания')
        if not project:
            raise GraphQLError('Проект должен быть задан')

        _type, _id = Node.from_global_id(project)
        assert_project_id(_type)
        project = m.Project.objects.get(id=_id)
        if appendix:
            _type, _id = Node.from_global_id(appendix)
            assert_attahcment_id(_type)
            appendix = m.Appendix.get(id=_id)
        print('appendix', appendix)

        # выбрать все бронирования, пересекающиеся с данным:
        #       (дата начала    < дата окончания периода и дата окончания >= дата начала периода)

        bad_sides_list = []
        bad_reservations = []
        bulk_reservation_list = []
        created_reservations = []
        updated_reservations = []
        changed_project = None
        changed_appendix = None

        if sides_ids:  # добавление броней
            sides_ids_list = []
            for side_id in sides_ids:
                _type, _id = Node.from_global_id(side_id)
                assert_construction_side_id(_type)
                if check_reservation_for_side_exists(_id, period_date_to, period_date_from):
                    bad_sides_list.append(side_id)
                else:
                    sides_ids_list.append(_id)
            sides = m.ConstructionSide.objects.filter(id__in=sides_ids_list)
            for side in sides:
                bulk_reservation_list.append(
                    m.Reservation(
                        construction_side=side,
                        date_to=period_date_to,
                        date_from=period_date_from,
                        branding=branding,
                        reservation_type=reservation_type,
                        project=project,
                    )
                )
                if project:
                    changed_project = project.id
            created = m.Reservation.objects.bulk_create(bulk_reservation_list)
            created_reservations = [Node.to_global_id('VReservationOptimizedNode', v.id) for v in created]
            if appendix:
                appendix.reservations.add(*created)
                changed_appendix = appendix.id

        if ids:
            reservation_ids_to_update = []
            updating_fields = ['date_from', 'date_to', 'project']
            has_appendix = 'appendix' in input
            has_branding = 'branding' in input
            has_reservation_type = 'reservation_type' in input
            if project:
                changed_project = project.id
            if has_appendix:
                updating_fields.append('appendix')
                changed_appendix = appendix.id
            if has_branding:
                updating_fields.append('branding')
            if has_reservation_type:
                updating_fields.append('reservation_type')

            for reservation_id in ids:
                _type, _id = Node.from_global_id(reservation_id)
                reservation = m.Reservation.objects.filter(id=_id).only('id', 'construction_side_id')
                if not reservation:
                    bad_reservations.append(reservation_id)
                else:
                    reservation = reservation[0]

                    if check_reservation_for_side_exists(
                        reservation.construction_side_id, period_date_to, period_date_from, _id
                    ):
                        bad_reservations.append(reservation_id)
                    else:
                        reservation.date_from = period_date_from
                        reservation.date_to = period_date_to
                        reservation.project = project
                        if has_appendix:
                            reservation.appendix = appendix
                        if has_branding:
                            reservation.branding = branding
                        if has_reservation_type:
                            reservation.reservation_type = reservation_type

                        reservation_ids_to_update.append(reservation)

            if reservation_ids_to_update:
                m.Reservation.objects.bulk_update(reservation_ids_to_update, updating_fields)
                updated_reservations = [
                    Node.to_global_id('VReservationOptimizedNode', v.id) for v in reservation_ids_to_update
                ]

        return CreateOrUpdateReservation(
            bad_sides=bad_sides_list,
            bad_reservations=bad_reservations,
            created_reservations=created_reservations,
            updated_reservations=updated_reservations,
            changed_project=Node.to_global_id('VProjectNode', changed_project) if changed_project is not None else None,
            changed_appendix=Node.to_global_id('VAppendixNode', changed_appendix) if changed_appendix is not None else None,
            ok=True,
        )
