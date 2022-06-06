IGNORE_PARAMS = [
    'fastSearch',
    'orderBy',
]


def parse_parameters(params: dict, ignore_params=[]):
    # Converts gql parameters names to pythonic
    converted_params = {}
    for key, value in params.items():
        if key in IGNORE_PARAMS or key in ignore_params:
            converted_params[key] = value
            continue
        new_name = str()
        for letter in key:
            if letter.isupper():
                new_name += f'_{letter.lower()}'
            else:
                new_name += letter
        converted_params[new_name] = value
    return converted_params
