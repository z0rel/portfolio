from typing import List, Dict

from django.db import transaction
import sys
import traceback

from django.core.management.base import BaseCommand, CommandError

from .args_search_location import get_args_search_location
from .args_to_create_location import get_args_to_create_location
from .get_db_addresses_of_rts_constructions import get_db_addresses_of_rts_constructions
from .group_locations_by_key import group_locations_by_key, T_GROUPS_OF_LOCATIONS
from .populate_db_data import populate_db_data, LocationDbData
from .populate_location_str_row import populate_str_row

from .update_location import update_location
from .._load_um_constructions_rts.merge_addresses import T_DB_ADDRESSES
from ..utils import (
    get_sheet_value,
    GetOrCreate,
    get_default_country,
    PATH_EXCEL_LOCATIONS,
    PATH_EXCEL_LOCATIONS_SHEET,
)


class Command(BaseCommand):
    help = 'Загрузить базу земельных участков'

    def add_arguments(self, parser):
        parser.add_argument(
            '--drop-unexisted',
            action='store_true',
            help='удалять местоположения присутствующие в файле, но отсутствуют в базе',
        )

    def handle(self, *args, **options):
        # TODO: вынести в аргумент
        need_drop = 'drop_unexisted' in options
        wb, cols, sheet, sheet_value = get_sheet_value(PATH_EXCEL_LOCATIONS, PATH_EXCEL_LOCATIONS_SHEET)

        batches = []
        get_or_create = GetOrCreate()
        from ptc_deco.api import models as m

        db_addresses: Dict[str, T_DB_ADDRESSES] = get_db_addresses_of_rts_constructions()
        handled_locations = {x.id: False for x in m.Location.objects.all()}

        with transaction.atomic():
            country = get_default_country(get_or_create)

            db_rows: List[LocationDbData] = [
                populate_db_data(populate_str_row(sheet_value, row_idx), get_or_create, country, db_addresses)
                for row_idx in range(2, sheet.max_row + 1)
            ]
            locations: T_GROUPS_OF_LOCATIONS = group_locations_by_key(db_rows)
            for loc_key, rv_list in locations.items():
                db_row = rv_list[0]
                try:
                    args_location_query = get_args_search_location(db_row)
                    args_location = get_args_to_create_location(db_row)

                    if args_location:
                        f_location = None
                        if args_location_query:
                            f_location = m.Location.objects.filter(**args_location_query)
                        if not f_location:
                            batches.append(m.Location(**args_location))
                        else:
                            for location in f_location:
                                update_location(location, db_row)

                except Exception as e:
                    print('Error in ', db_row.r.row_idx, ':', str(e))
                    print(
                        ' '.join(
                            [str(db_row.r.row_idx)]
                            + [str(sheet.cell(db_row.r.row_idx, k).value) for k in range(1, sheet.max_column)]
                        )
                    )
                    traceback.print_exc(file=sys.stdout)
                    raise

            if batches:
                m.Location.objects.bulk_create(batches)

        if need_drop:
            m.Location.objects.filter(id__in=[k for (k, handled) in handled_locations.items() if not handled]).delete()
