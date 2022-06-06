import sys
from itertools import chain
from typing import Optional, List, Dict, Set, Any, Tuple

from django.db import transaction
from django.core.management.base import BaseCommand, CommandError
from django.db.models import Q, Count, Model, F
from tqdm import trange

from ptc_deco.api import models as m
from .args_to_create_rts_construction import get_args_to_create_rts_construction
from .group_rts_constructions_by_sides import group_rts_constructions_by_sides
from .load_or_create_location_for_construction_rts import load_or_create_location_for_construction_rts
from .load_or_update_construction_side import load_or_update_construction_side, T_CONSTRUCTION_SIDE_BATCHES
from .populate_addresses_from_str import populate_addreses_from_str
from .types import T_LIST_OF_PARSED_CONSTRUCTIONS, T_LIST_HANDLED, T_LIST_OF_RTS_DB_ROW

from ..utils import (
    GetOrCreate,
    filter_dict,
    generate_key,
    filter_dict_ordered,
    PATH_EXCEL_CONSTRUCTIONS,
    PATH_EXCEL_CONSTRUCTIONS_SHEET_RTS,
    get_sheet_value,
)

from .merge_city_district_postcode import (
    merge_city_district_postcode,
    objects_by_title,
    check_duplicates,
    TD_CITIES,
    TD_POSTCODES,
    TD_DISTRICTS2,
    TD_DISTRICTS,
    TD_POSTCODES2,
)

from .populate_rts_row_from_sheet import ConstructionRtsStrRow, populate_rts_row_from_sheet
from .populate_rts_row_db_values import ConstructionRtsRowDbValues, populate_db_row, check_row_is_empty
from .merge_addresses import merge_addresses
from .check_and_update_construction_side_format import check_and_update_construction_side_format
from .update_construction import update_construction
from .update_location import update_location
from .update_construction_side import update_construction_side
from .parsed_constructions import ParsedConstruction


def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


class Command(BaseCommand):
    help = 'Загрузить базу конструкци (УМ)'

    def add_arguments(self, parser):
        parser.add_argument(
            '--drop-unexisted',
            action='store_true',
            help='удалять конструкции и их стороны, которые есть в файле, но отсутствуют в базе',
        )

    def populate_rts_row(self, sheet, cols, m, get_or_create: GetOrCreate, start_indices) -> T_LIST_OF_RTS_DB_ROW:
        """
        @type get_or_create: GetOrCreate
        @type rts_rows: list
        @type m: ptc_deco.api.models
        """

        bulk_create_locations: List[Tuple[m.Location, ConstructionRtsRowDbValues]] = []
        populated_rts_rows: List[ConstructionRtsRowDbValues] = []
        cached_xlsx_rows: List[ConstructionRtsStrRow]

        (
            cached_xlsx_rows,
            sheet_value,
            db_cities,
            db_districts2,
            db_postcodes2,
            db_addresses,
        ) = populate_addreses_from_str(sheet, cols, get_or_create)

        try:
            l_addresses = m.Addresses.objects.filter(
                address__in=set(
                    chain(
                        [r.s_address_legal for r in cached_xlsx_rows],
                        [r.s_address_market for r in cached_xlsx_rows],
                    )
                )
            )
            addrdict = {}
            for addr in l_addresses:
                try:
                    addrdict[addr.address].append(addr)
                except KeyError:
                    addrdict[addr.address] = [addr]

            all_locations = m.Location.objects.all().annotate(
                legal_address_title=F('legal_address__address'),
                marketing_address_title=F('marketing_address__address'),
                district_id=F('postcode__district_id'),
            )
            locations_by_cadastral_number = {}
            locations_by_partial_key = {}
            locations_to_create_by_key = {}
            for _l in all_locations:
                partial_key = (
                    _l.cadastral_number,
                    _l.marketing_address_title.lower() if _l.marketing_address_title is not None else None,
                )
                try:
                    locations_by_cadastral_number[partial_key].append(_l)
                except KeyError:
                    locations_by_cadastral_number[partial_key] = [_l]

                partial_key = (
                    _l.legal_address_title.lower() if _l.legal_address_title is not None else None,
                    _l.marketing_address_title.lower() if _l.marketing_address_title is not None else None,
                    _l.district_id,
                    _l.family_construction_id,
                )
                try:
                    locations_by_partial_key[partial_key].append(_l)
                except KeyError:
                    locations_by_cadastral_number[partial_key] = [_l]

            for r, _ in zip(
                cached_xlsx_rows, trange(len(cached_xlsx_rows), desc='populated sheet row Construction RTS')
            ):
                db_row: Optional[ConstructionRtsRowDbValues] = populate_db_row(
                    get_or_create, r, db_cities, db_districts2, db_postcodes2, addrdict, start_indices
                )
                if db_row is not None:
                    populated_rts_rows.append(db_row)
                    load_or_create_location_for_construction_rts(
                        db_row,
                        locations_by_cadastral_number,
                        locations_by_partial_key,
                        bulk_create_locations,
                        locations_to_create_by_key,
                    )

        except Exception as e:
            print(sheet_value.RESULT)
            raise

        print(sheet_value.RESULT)

        if bulk_create_locations:
            created_locations = m.Location.objects.bulk_create(
                [location for (location, partial_key) in bulk_create_locations]
            )
            for location, partial_key in zip(
                created_locations, [partial_key for (location, partial_key) in bulk_create_locations]
            ):
                for db_row in locations_to_create_by_key[partial_key]['rows']:
                    db_row.location = location

        return populated_rts_rows

    def populate_constructions(
        self, get_or_create: GetOrCreate, rts_rows_with_location: T_LIST_OF_RTS_DB_ROW, need_drop
    ):
        """

        @type
        """

        def add_arg_to_dict_if_is_not_none(src, dst_dict: Dict, attr):
            val = getattr(src, attr, None)
            if val is not None:
                dst_dict[attr] = val

        r: ConstructionRtsStrRow
        rv: ConstructionRtsRowDbValues
        location: m.Location
        bulk_create_construction = []
        bulk_zip_db_idx = []
        construction_side_batches: T_CONSTRUCTION_SIDE_BATCHES = {}

        handled_construction_sides: Dict[int, bool] = {
            x.id: False for x in m.ConstructionSide.objects.filter(construction__is_nonrts=False)
        }
        handled_construction = {x.id: False for x in m.Construction.objects.filter(is_nonrts=False)}

        constructions = group_rts_constructions_by_sides(rts_rows_with_location)
        rv_list: List[ConstructionRtsRowDbValues]
        for (rv_key, rv_list), _ in zip(
            constructions.items(), trange(len(constructions), desc='populated construction rts row')
        ):
            rv = rv_list[0]
            args_for_creating_construction = get_args_to_create_rts_construction(rv_list)

            if args_for_creating_construction:
                # Уточнить, что с ними делать
                query_kwargs = {}
                add_arg_to_dict_if_is_not_none(rv.r, query_kwargs, 'tech_invent_number')
                add_arg_to_dict_if_is_not_none(rv.r, query_kwargs, 'buh_invent_number')

                # construction: m.Construction = None
                it_filtered_constructions = None
                if query_kwargs:
                    # add_arg_to_dict_if_is_not_none(rv, query_kwargs, 'location')
                    # add_arg_to_dict_if_is_not_none(rv, query_kwargs, 'format')
                    query_kwargs['is_nonrts'] = False
                    it_filtered_constructions = m.Construction.objects.filter(**query_kwargs)
                if not it_filtered_constructions:
                    bulk_create_construction.append(m.Construction(**args_for_creating_construction))
                    bulk_zip_db_idx.append((rv, rv_key))
                # Фильтруем только конструкции с заданными номерами
                elif 'tech_invent_number' in query_kwargs or 'buh_invent_number' in query_kwargs:
                    if len(it_filtered_constructions) > 1:
                        print(
                            f'Error for Constructions RTS key: {query_kwargs}: len > 1: {len(it_filtered_constructions)}'
                        )
                        for c in it_filtered_constructions:
                            print(c)
                    for _construction in it_filtered_constructions:
                        update_construction(handled_construction, _construction, rv.r, rv, rv_list)

                    # конструкция обновлена - обновить стороны конструкции
                    load_or_update_construction_side(
                        get_or_create,
                        it_filtered_constructions[0],
                        rv_list,
                        construction_side_batches,
                        handled_construction_sides,
                    )

        if bulk_create_construction:
            print(f'Bulk created contruction {len(bulk_create_construction)}')
            created = m.Construction.objects.bulk_create(bulk_create_construction)
            for i, (_construction, (db_row, rv_key)) in enumerate(zip(created, bulk_zip_db_idx)):
                if _construction is None:
                    print(f'Error: created construction rts is None on idx: {i}')
                    continue

                # конструкция создана - создать или обновить стороны конструкции
                load_or_update_construction_side(
                    get_or_create,
                    _construction,
                    constructions[rv_key],
                    construction_side_batches,
                    handled_construction_sides,
                )

        construction_side_batches_list = [v[0][0] for v in construction_side_batches.values()]
        if construction_side_batches_list:
            m.ConstructionSide.objects.bulk_create(construction_side_batches_list)

        if need_drop:
            m.ConstructionSide.objects.filter(
                id__in=[k for (k, handled) in handled_construction_sides.items() if not handled]
            ).delete()

            m.Construction.objects.filter(
                id__in=[k for (k, handled) in handled_construction.items() if not handled]
            ).delete()

    def handle(self, *args, **options):
        need_drop = 'drop_unexisted' in options

        wb, cols, sheet, sheet_value = get_sheet_value(PATH_EXCEL_CONSTRUCTIONS, PATH_EXCEL_CONSTRUCTIONS_SHEET_RTS)

        get_or_create = GetOrCreate()
        start_indices: Dict[str, int] = {}  # loc_code: start loc code

        with transaction.atomic():
            try:
                rts_db_rows: T_LIST_OF_RTS_DB_ROW = self.populate_rts_row(sheet, cols, m, get_or_create, start_indices)

                rts_rows: T_LIST_OF_PARSED_CONSTRUCTIONS
                unhandled_constructions: T_LIST_HANDLED
                self.populate_constructions(get_or_create, rts_db_rows, need_drop)
            except transaction.TransactionManagementError as exc:
                print('transaction error', str(exc))
                return
