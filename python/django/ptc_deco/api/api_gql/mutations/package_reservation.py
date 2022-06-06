import graphene
from graphene import ID, List, Boolean
from graphql import GraphQLError
from graphene.relay import Node
from django.db.models import Q
from ....api import models as m
from .estimate_crud.estimate_utils import (
    assert_reservation_type_msg,
    assert_package_reservation_msg
)
from ..utils.auth.decorators import login_or_permissions_required


# Существующая бронь с указанными типами будет игнорироваться
IGNORED_RESERVATION_TYPES = [
    m.ReservationType.ReservationTypeInteger.FREE,
    m.ReservationType.ReservationTypeInteger.RESERVED,
    m.ReservationType.ReservationTypeInteger.CANCELLED,
]


def check_reservation_for_side_exists(construction_side, period_date_to, period_date_from):
    return m.Reservation.objects.filter(
        ~Q(reservation_type__ikey__in=IGNORED_RESERVATION_TYPES),
        date_from__lt=period_date_to,
        date_to__gte=period_date_from,
        construction_side=construction_side,
    ).exists()


class CreateReservationPackage(graphene.Mutation):
    class Arguments:
        id = graphene.ID(description='Идентификатор пакета', required=True)
        branding = graphene.Boolean(description='Брендинг', required=True)
        date_from = graphene.DateTime(description='Дата начала', required=True)
        date_to = graphene.DateTime(description='Дата окончания', required=True)
        reservation_type_id = graphene.ID(description='Идентификатор типа резервирования', required=True)
        project_id = graphene.ID(description='Идентификатор проекта', required=True)
        appendix_id = graphene.ID(description='Идентификатор приложения к договору')

    ok = Boolean()
    created_reservations = List(ID, description='Идентификаторы созданных броней')
    reservation_id = graphene.ID(description='Идентификатор брони пакета')
    bad_sides_count = graphene.Int(description='Количество сторон, для которых не установлена бронь')
    bad_sides = graphene.List(ID, description='Стороны, для которых не установлена бронь')

    @login_or_permissions_required(login_required=False, permissions=('api.add_reservationpackage', ))
    def mutate(root, info, **input):
        package_id = input.get('id', None)
        branding = input.get('branding', None)
        date_from = input.get('date_from', None)
        date_to = input.get('date_to', None)
        reservation_type_id = input.get('reservation_type_id', None)
        project_id = input.get('project_id', None)
        appendix_id = input.get('appendix_id', None)

        if date_from > date_to:
            raise GraphQLError(f'Дата окончания не может быть позднее даты начала')

        reservation_date_range = [date_from, date_to]

        package = m.Package.objects.filter(id=package_id).first()

        if not package:
            raise GraphQLError(f'Пакет с идентификатором {package_id} не существует')

        package_reserved = m.ReservationPackage.objects.filter(
            Q(package=package) &
            Q(
                Q(date_from__range=reservation_date_range) |
                Q(date_to__range=reservation_date_range)
            ) &
            ~Q(reservation_type__ikey__in=IGNORED_RESERVATION_TYPES)
        ).exists()

        if package_reserved:
            raise GraphQLError(f'Пакет с идентификатором {package_id} с {date_from} по {date_to} уже зарезервирован')

        _type, _id = Node.from_global_id(reservation_type_id)
        assert_reservation_type_msg(_type)
        reservation_type = m.ReservationType.objects.filter(id=_id).first()
        if not reservation_type:
            raise GraphQLError(f'Типа бронирования с идентификатором {reservation_type_id} не существует')

        sides = m.ConstructionSide.objects.filter(package=package)

        free_sides = []
        bad_sides = []
        for side in sides:
            if check_reservation_for_side_exists(side, date_to, date_from):
                bad_sides.append(side)
            else:
                free_sides.append(side)

        if not free_sides:
            raise GraphQLError(f'В пакете нет сторон, для которых не создана бронь')

        _type, _id = Node.from_global_id(project_id)
        project = m.Project.objects.filter(id=_id).first()
        if not project:
            raise GraphQLError(f'Проекта с идентификатором {project_id} не существует')

        appendix = None
        if appendix_id:
            _type, _id = Node.from_global_id(appendix_id)
            appendix = m.Appendix.filter(id=_id).first()
            if not appendix:
                raise GraphQLError(f'Приложения к договору с таким идентификатором {appendix_id} не существует')

        package_reservation = m.ReservationPackage.objects.create(
            package=package,
            branding=branding,
            date_from=date_from,
            date_to=date_to,
            project=project,
            reservation_type=reservation_type,
            appendix=appendix,

        )

        bulk_reservation_list = []
        for side in free_sides:
            bulk_reservation_list.append(
                m.Reservation(
                    construction_side=side,
                    date_from=date_from,
                    date_to=date_to,
                    branding=branding,
                    reservation_type=reservation_type,
                    project=project,
                    reservation_package=package_reservation
                )
            )
        created = m.Reservation.objects.bulk_create(bulk_reservation_list)
        created_reservations = [Node.to_global_id('VReservationOptimizedNode', v.id) for v in created]

        reservation_id = Node.to_global_id('VPackageReservationOptimizedNode', package_reservation.id)

        return CreateReservationPackage(
            ok=True,
            reservation_id=reservation_id,
            created_reservations=created_reservations,
            bad_sides_count=len(bad_sides),
            bad_sides=[Node.to_global_id('VConstructionSideOptimizedNode', side.id) for side in bad_sides],
        )


class UpdatePackageReservationType(graphene.Mutation):
    class Arguments:
        id = graphene.ID(description='Идентификатор брони пакета')
        reservation_type_id = graphene.ID(description='Идентификатор типа бронирования')

    ok = graphene.Boolean()

    @login_or_permissions_required(login_required=False, permissions=('api.change_reservationtype', ))
    def mutate(root, info, **input):
        package_reservation_id = input.get('id', None)
        _type, _id = Node.from_global_id(package_reservation_id)
        assert_package_reservation_msg(_type)
        package_reservation = m.ReservationPackage.objects.filter(id=_id).first()
        if not package_reservation:
            raise GraphQLError(f'Брони пакета с идентификатором {package_reservation_id} не существует')

        reservation_type_id = input.get('reservation_type_id', None)
        _type, _id = Node.from_global_id(reservation_type_id)
        assert_reservation_type_msg(_type)
        reservation_type = m.ReservationType.objects.filter(id=_id).first()
        if not reservation_type:
            raise GraphQLError(f'Типа бронирования с идентификатором {reservation_type_id} не существует')

        package_reservation.reservation_type = reservation_type

        package_side_reservations = m.Reservation.objects.filter(reservation_package=package_reservation)
        if reservation_type != m.ReservationType.ReservationTypeInteger.SALED:
            package_side_reservations.update(reservation_type=reservation_type)

        package_reservation.save()

        return UpdatePackageReservationType(ok=True)
