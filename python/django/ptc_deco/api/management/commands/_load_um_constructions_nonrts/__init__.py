from typing import Dict, Tuple, List

from django.db import transaction
from django.core.management.base import BaseCommand
from tqdm import trange

from ptc_deco.api import models as m
from .args_to_create_nonrts_construction import get_args_to_create_nonrts_construction

from .args_search_nonrts_constructions import get_args_to_search_nonrts_construction
from .get_annotated_all_addresses import get_annotated_all_addresses, T_ANNOTATED_ADDRESSES, T_DICT_DB_ADDRESSES
from .group_nonrts_constructions_by_sides import group_nonrts_constructions_by_sides
from .handle_family_hierarchies_for_side import create_family_resolver, handle_tr_family_hierarchy_error

from .populate_nonrts_row_db_values import ConstructionNonRtsDbValues, populate_nonrts_row_db_values

from .populate_nonrts_row_from_sheet import ConstructionNonRtsStrRow, populate_nonrts_row_from_sheet
from .update_nonrts_construction import update_nonrts_construction
from .._load_um_constructions_rts.load_or_update_construction_side import (
    load_or_update_construction_side,
    T_CONSTRUCTION_SIDE_BATCHES,
)
from ..utils import (
    GetOrCreate,
    get_families_tree,
    get_sheet_value,
    PATH_EXCEL_CONSTRUCTIONS,
    PATH_EXCEL_CONSTRUCTIONS_SHEET_NONRTS,
)


class Command(BaseCommand):
    help = 'Загрузить базу конструкци (УМ, НОНРТС)'

    def add_arguments(self, parser):
        parser.add_argument(
            '--drop-unexisted',
            action='store_true',
            help='удалять конструкции и их стороны, присутствующие в файле, но отсутствующие в базе',
        )

    def handle(self, *args, **options):
        need_drop = 'drop_unexisted' in options
        wb, cols, sheet, sheet_value = get_sheet_value(
            PATH_EXCEL_CONSTRUCTIONS, PATH_EXCEL_CONSTRUCTIONS_SHEET_NONRTS, 2
        )

        get_or_create = GetOrCreate()

        addresses_by_address: T_DICT_DB_ADDRESSES = get_annotated_all_addresses()

        with transaction.atomic():
            country = get_or_create(m.Country, title='Республика Казахстан')
            nonrts_row = []
            families_tree = get_families_tree()

            for row_idx in range(3, sheet.max_row + 1):
                r: ConstructionNonRtsStrRow = populate_nonrts_row_from_sheet(row_idx, sheet_value, cols)
                rv: ConstructionNonRtsDbValues = populate_nonrts_row_db_values(
                    get_or_create, country, families_tree, addresses_by_address, r
                )
                nonrts_row.append(rv)

            constructions: Dict[Tuple, List[ConstructionNonRtsDbValues]] = group_nonrts_constructions_by_sides(
                nonrts_row
            )

            handled_construction_sides = {
                x.id: False for x in m.ConstructionSide.objects.filter(construction__is_nonrts=True)
            }
            handled_constructions = {x.id: False for x in m.Construction.objects.filter(is_nonrts=True)}

            bulk_constructions = []
            construction_side_batches: T_CONSTRUCTION_SIDE_BATCHES = {}
            rv_list: List[ConstructionNonRtsDbValues]

            family_resolver = create_family_resolver(families_tree)

            for (rv_key, rv_list), _ in zip(
                constructions.items(), trange(len(constructions), desc='populated construction nonrts row')
            ):
                args_to_create_construction = get_args_to_create_nonrts_construction(rv_key, rv_list)

                if args_to_create_construction:
                    filter_args = get_args_to_search_nonrts_construction(rv_key)
                    it_filtered_constructions = m.Construction.objects.filter(**filter_args)
                    if not it_filtered_constructions:
                        construction = m.Construction(**args_to_create_construction)
                        bulk_constructions.append((construction, rv_key))
                    else:
                        for construction in it_filtered_constructions:
                            update_nonrts_construction(handled_constructions, construction, rv_list)

                        if len(it_filtered_constructions) > 1:
                            print(
                                f"""Error: nonrts constructions for key {filter_args} is > 1: {
                                    len(it_filtered_constructions)}"""
                            )
                            for c in it_filtered_constructions:
                                print('--', c)

                        load_or_update_construction_side(
                            get_or_create,
                            it_filtered_constructions[0],
                            rv_list,
                            construction_side_batches,
                            handled_construction_sides,
                            _resolve_family_hierarchy=family_resolver,
                            _handle_tr_family_hierarchy_error=handle_tr_family_hierarchy_error,
                        )

            if bulk_constructions:
                _to_create = [construction for (construction, key) in bulk_constructions]
                _keys = [key for (construction, key) in bulk_constructions]

                created = m.Construction.objects.bulk_create(_to_create)
                for construction, key in zip(created, _keys):
                    rv_list = constructions[key]
                    load_or_update_construction_side(
                        get_or_create,
                        construction,
                        rv_list,
                        construction_side_batches,
                        handled_construction_sides,
                        _resolve_family_hierarchy=family_resolver,
                        _handle_tr_family_hierarchy_error=handle_tr_family_hierarchy_error,
                    )

            construction_side_batches_list = [v[0][0] for v in construction_side_batches.values()]
            if construction_side_batches_list:
                m.ConstructionSide.objects.bulk_create(construction_side_batches_list)

        if need_drop:
            m.ConstructionSide.objects.filter(
                id__in=[k for (k, handled) in handled_construction_sides.items() if not handled]
            ).delete()
            m.Construction.objects.filter(
                id__in=[k for (k, handled) in handled_constructions.items() if not handled]
            ).delete()
