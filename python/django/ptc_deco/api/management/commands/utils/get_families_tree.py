_FORMAT_CODES = {
    'ситилайт decaux': 'DEC',
    'ситилайт decaux mupi': 'MUP',
    'ситилайт decaux флагшток': 'FLA',
    'ситилайт аксиома': 'AXI',
    'антивандальная': 'ANT',
    'подкартник': 'INF',
    'ситилайт': 'MLB',
    'двойной ситилайт': 'BLB',
    'сениор': 'SEN',
    'magic mupi': 'MAG',
    'led-дисплей': 'LED',
    'стеклянная': 'GLA',
}


def get_families_tree():
    def gen_adv_side(src, additionals=None):
        return {
            prefix: (
                [
                    *(
                        [(f'{prefix}{i}', f'{letter}{i}') for i in range(1, cnt + 1)]
                        if cnt is not None and isinstance(cnt, int) and cnt > 0
                        else []
                    ),
                    *(
                        [(prefix, letter)] + [(f'{prefix}{i}', f'{letter}{i}') for i in range(1, cnt + 1)]
                        if cnt is not None and isinstance(cnt, int) and cnt < 0
                        else []
                    ),
                    *([(prefix, letter)] if cnt is None else []),
                    *(additionals if additionals is not None else []),
                ],
                size,
                letter,
            )
            for (prefix, letter, cnt, size) in src
        }

    def gen_combined(args):
        dst = {}
        for prefix, letter, cnt, size_val in args:
            d = gen_adv_side(
                [(f'{prefix} {letter}', letter, cnt, size_val)],
                [
                    (f'{prefix} A/S', 'А/S'),
                    (f'{prefix} A/N', 'А/N'),
                    (f'{prefix} A/Z', 'А/Z'),
                    (f'{prefix} A/V', 'A/V'),
                ],
            )
            dst.update(d)
        return dst

    def sides_static_other4snzv(size):
        return gen_combined([('Статичная', 'A', -4, size), ('Статичная', 'В', -4, size)])

    def sides_citylight_decaux(size):
        return gen_combined(
            [
                ('Скроллерная', 'A', 4, size),
                ('Скроллерная', 'В', 4, size),
                ('Статичная', 'A', -4, size),
                ('Статичная', 'В', -4, size),
            ]
        )

    def set_format_code(fmt):
        return fmt, _FORMAT_CODES[fmt.lower()]

    return {
        'Сениор': {
            'Европейские': {
                k: {
                    set_format_code('Сениор'): gen_adv_side(
                        [
                            ('Скроллерная A', 'A', 5, '314х230'),
                            ('Скроллерная В', 'В', 5, '314х230'),
                            ('Статичная A', 'A', None, '314х230'),
                            ('Статичная В', 'В', None, '314х230'),
                        ]
                    )
                }
                for k in ['Престиж', 'ГИП 2009']
            }
        },
        'Мюпи': {
            'Европейские': {
                'CIP Forum': {
                    set_format_code('Ситилайт Decaux MUPI'): gen_adv_side(
                        [
                            ('Скроллерная A', 'A', 4, '119х175'),
                            ('Скроллерная В', 'В', 4, '119х175'),
                            ('Статичная A', 'A', None, '119х175'),
                            ('Статичная В', 'В', None, '119х175'),
                        ]
                    )
                },
                'CIP Szekely': {
                    set_format_code('Ситилайт Decaux MUPI'): gen_adv_side(
                        [
                            ('Скроллерная A', 'A', 4, '115х168'),
                            ('Скроллерная В', 'В', 4, '120х173'),
                            ('Статичная A', 'A', None, '115х168'),
                            ('Статичная В', 'В', None, '120х173'),
                        ]
                    )
                },
            }
        },
        'Флагшток': {
            'Европейские': {
                'CIP Forum': {
                    set_format_code('Ситилайт Decaux Флагшток'): gen_adv_side(
                        [
                            ('Статичная A', 'A', -2, '119х175'),
                            ('Статичная В', 'В', None, '119х175'),
                        ]
                    )
                }
            }
        },
        'Остановка': {
            'Европейские': {
                **{
                    k: {
                        set_format_code('Ситилайт Decaux'): sides_citylight_decaux('119х175'),
                        set_format_code('Подкартник'): sides_static_other4snzv('90х80'),
                    }
                    for k in [
                        'В1',
                        'А0',
                        'А1',
                        'А2 двойная',
                        'А2 полуторная',
                        'А2',
                        'Китай',
                        'Кокс',
                        'Мерак',
                        'РТС5',
                        'РТС6',
                        'РТС7',
                        'РТС8',
                        'Синтези',
                        'Турция',
                        'Фостер двойной',
                        'Фостер одинарный',
                        'Франция',
                        'Цезарь',
                        'Швеция1',
                        'Швеция2',
                        'Штерн',
                    ]
                },
                **{
                    'РТС4 - Ситилайт Decaux': {
                        set_format_code('Ситилайт Decaux'): sides_citylight_decaux('119х175'),
                        set_format_code('Двойной Ситилайт'): sides_static_other4snzv('184х240'),
                        set_format_code('Подкартник'): sides_static_other4snzv('90х80'),
                    }
                },
            },
            'Старые': {
                **{
                    'Б1': {set_format_code('Антивандальная'): sides_static_other4snzv('120х180')},
                    'Б1 - Антивандальная': {set_format_code('Антивандальная'): sides_static_other4snzv('120х180')},
                },
                **{
                    k: {
                        set_format_code('Ситилайт'): sides_static_other4snzv('120х180'),
                        set_format_code('Двойной Ситилайт'): sides_static_other4snzv('184х244'),
                        set_format_code('Подкартник'): sides_static_other4snzv('90х80'),
                        set_format_code('Антивандальная'): sides_static_other4snzv('120х180'),
                    }
                    for k in ['РТС1', 'РТС1/L', 'РТС1/K', 'РТС3']
                },
                **{
                    'Артс': {
                        set_format_code('Ситилайт'): sides_static_other4snzv('128х174'),
                        set_format_code('Ситилайт Аксиома'): sides_static_other4snzv('120х180'),
                        set_format_code('Подкартник'): sides_static_other4snzv('128х174'),
                    }
                },
            },
        },
    }
