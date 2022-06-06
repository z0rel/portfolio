from graphql import GraphQLError


def err_msg(item, _type):
    return f"""Передан неверный тип для идентификатора {item}: {_type
    }, должен быть VReservationCalculatedNode | VEstimateReservationsNonRtsNode |{""
    } VEstimateNoSidesNonRtsNode | VEstimateAdditionalCostsRtsNode"""


def assert_msg(_type, _wait_type, entity):
    if _type != _wait_type:
        raise GraphQLError(
            f"""Передан идентификатор {entity}, не являющийся типом {entity} - {_type
        }, должен быть {_wait_type}"""
        )


def assert_msg_list(_type, _wait_type, entity):
    if _type not in _wait_type:
        raise GraphQLError(
            f"""Передан идентификатор {entity}, не являющийся типом {entity} - {_type
        }, должен быть из {' '.join(_wait_type)}"""
        )


def assert_attahcment_id(_type):
    assert_msg(_type, 'VAppendixOptimizedNode', 'приложения')


def assert_project_id(_type):
    assert_msg_list(_type, ['VProjectOptimizedNode', 'VProjectNode'], 'бронирования')


def page_item_err_msg(estimate_section):
    return f"""Передан неверный тип выбранного пункта таблицы {estimate_section
    }, должен быть reservations-rts | additional-rts | non-rtc"""


def assert_construction_side_id(_type):
    assert_msg(_type, 'VConstructionSideOptimizedNode', 'стороны конструкции')


def assert_reservation_type_msg(_type):
    assert_msg_list(_type, ['VReservationTypeOptimizedNode', 'VReservationTypeNode'], 'бронирования')


def assert_reservation_msg(_type):
    assert_msg(_type, 'VReservationOptimizedNode', 'бронирования')


def assert_package_reservation_msg(_type):
    assert_msg(_type, 'VPackageReservationOptimizedNode', 'бронирования')
