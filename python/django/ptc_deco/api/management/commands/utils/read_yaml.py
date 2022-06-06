import os

import yaml


def read_yaml(name_file: str) -> dict:
    # все пути относительно current working direcotry
    path_file = os.path.join(os.path.dirname('.'), 'datasource', f'{name_file}')
    try:
        with open(os.path.join(path_file), 'r', encoding='utf-8') as f:
            data = yaml.safe_load(f)
    except FileNotFoundError:
        message = f'===========  NotFound  {path_file} ============='
        raise FileNotFoundError(message)
    else:
        return data
