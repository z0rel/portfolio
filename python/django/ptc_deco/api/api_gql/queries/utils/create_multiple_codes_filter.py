from typing import List
from django.db.models.expressions import Q


def create_filter(expr: Q, codes: List[dict]):
    if codes:
        for code in codes:
            q_code = Q()
            for parameter_name, parameter_value in code.items():
                q_code &= Q(**{f'{parameter_name}': parameter_value})
            expr |= q_code
    return expr
