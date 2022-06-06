from ...generate_order_by_class import generate_order_by_class


def field_in_construction(field=None):
    return [
        'reservation__construction_side__construction' + f'__{field}' if field else '',
        'construction_side__construction' + f'__{field}' if field else '',
        'construction' + f'__{field}' if field else '',
    ]


def field_in_construction_side(field):
    return [
        'reservation__construction_side__' + field,
        'construction_side__' + field,
        'reservation__construction_side',
        'construction_side',
    ]


# fmt: off
ORDERING_FIELDS = {
  'date':                   (['start_mounting', 'end_mounting'], 'По дате начала и окончания', []),
  'start_mounting':         (['start_mounting'],         'По дате начала монтажа', []),
  'end_mounting':           (['end_mounting'],           'По дате окончания монтажа', []),
  'range':                  (['mounting_range'],         'По приоритету', []),
  'mounting_task_title':    (['mounting_task_title'],    'По названию задачи', []),
  'comment':                (['comment'],                'По комментарию к монтажу', []),
  'mounting_done':          (['mounting_done'],          'По статусу "монтаж выполнен" / "монтаж не выполнен"', []),
  'unmounting_done':        (['unmounting_done'],        'По статусу "демонтаж выполнен" / "демонтаж не выполнен"', []),

  'family_title':           (['family_title'],           'По названию семейства', field_in_construction('model__underfamily__family__title')),
  'underfamily_title':      (['underfamily_title'],      'По названию подсемейства', field_in_construction('model__underfamily__title')),
  'model_title':            (['model_title'],            'По названию модели', field_in_construction('model__title')),
  'format_title':           (['format_title'],           'По названию формата', [
      *field_in_construction_side('advertising_side__side__format'),
  ]),
  'advertising_side_title': (['advertising_side_title'], 'По названию рекламной стороны', field_in_construction_side('advertising_side')),
  'side_title':             (['side_title'],             'По названию стороны', field_in_construction_side('advertising_side__side')),
  'side_size':              (['side_size'],              'По размеру стороны', [
      *field_in_construction_side('advertising_side__side')
  ]),

  'design_title':           (['design_title'],           'По названию дизайна', ['design']),
  'unmounting_design_title': (['unmounting_design_title'], 'По названию демонтируемого дизайна', ['unmounting_design']),
  'crew_num':               (['crew_num'],               'По номеру экипажа', ['crew']),
  'crew_name':              (['crew_name'],              'По имени представителя экипажа', ['crew']),
  'crew_phone':             (['crew_phone'],             'По телефону экипажа', ['crew']),
  'crew_city':              (['crew_city'],              'По городу экипажа', ['crew']),

  'project_title':          (['project_title'],          'По названию проекта', ['reservation__project']),
  'project_code':           (['project_code'],           'По коду проекта', ['reservation__project']),
  'appendix_code':          (['appendix_code'],          'По коду проекта', ['reservation__appendix']),
  'construction_side_code': ([*field_in_construction('location__postcode__title'),
                              *field_in_construction('num_in_district'),
                              'format_code',
                              'side_code',
                              'advertising_side_code'
                              ], 'По коду конструкции',
                             list(sorted({*field_in_construction('location__marketing_address'),
                                          *field_in_construction_side('advertising_side__side__format'),
                                          *field_in_construction('location__postcode')
                                          }))),
  'photo_names':            (['photo_names'], 'По № фото', ''),
  'downloaded_early':       (['downloaded_early'], 'Выгружено ранее', []),
  'isNonRts':               (field_in_construction('is_nonrts'), 'РТС/НОН РТС', field_in_construction()),
  'statusConnection':       (field_in_construction('status_connection'), 'По наличию освещения', field_in_construction()),
  'numInDistrict':          (field_in_construction('num_in_district'),                          'По номеру в районе', field_in_construction()),
  'postcodeTitle':          (field_in_construction('location__postcode__title'),                'По почтовому коду', field_in_construction('location__postcode')),
  'districtTitle':          (field_in_construction('location__postcode__district__title'),      'По названию района', field_in_construction('location__postcode__district')),
  'cityTitle':              (field_in_construction('location__postcode__district__city__title'), 'По городу', field_in_construction('location__postcode__district__city')),
  'addressMarketing':       (field_in_construction('location__marketing_address__address'), 'По маркетинговому адресу', field_in_construction('location__marketing_address')),
  'addressLegal':           (field_in_construction('location__legal_address__address'),     'По юридическому адресу', field_in_construction('location__legal_address')),
}
# fmt: on


GraphqlOrderBy = generate_order_by_class('MountingOrderBy', ORDERING_FIELDS)
