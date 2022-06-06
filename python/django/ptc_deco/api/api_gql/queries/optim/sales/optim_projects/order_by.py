from django.db.models import F

# fmt: off
ORDERING_FIELDS = {
    'code':                      (['start_date__year', 'num_in_year'], 'По коду проекта'),
    'title':                     (['title'], 'По названию проекта'),
    'brand_title':               (['brand__title'], 'По названию бренда'),
    'start_date':                (['start_date'], 'По дате начала проекта'),
    'created_at':                (['created_at'], 'По дате создания проекта'),
    'city_title':                (['cities_list', 'client_city_title', 'agency_city_title'], 'По городу', ),
    'client_title':              (['client_title'], 'По наименованию рекламодателя'),
    'agency_title':              (['agency_title'], 'По наименованию рекламного агентства'),
    'working_sector_title':      (['working_sector_title'], 'По сектору деятельности'),
    'back_office_manager_title': (['back_office_manager_last_name', 'back_office_manager_first_name'],
                                  'По менеджеру бек-оффиса'),
    'sales_manager_title':       (['sales_manager_last_name', 'sales_manager_first_name'], 'По менеджеру по продажам')
}
# fmt: on
