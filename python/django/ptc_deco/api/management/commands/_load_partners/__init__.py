from typing import List, Dict, Tuple

from django.db import transaction
from django.core.management.base import BaseCommand

from ptc_deco.api import models as m

from .args_search_partners import get_args_to_search_partners
from .args_to_create_partner import get_args_to_create_partner
from .group_partners_by_key import (
    T_GROUPS_OF_PARTNERS,
    group_partners_by_key,
    T_GROUPS_OF_PARTNERS_KEY,
    T_GROUPS_OF_PARTNERS_VALUES,
)
from .populate_partner_row_from_db_values import PartnerDbValues, populate_partner_row_from_db_values
from .populate_partner_row_from_sheet import PartnerStrRow, populate_partner_row_from_sheet
from .update_partner import update_partner
from .update_partner_linked_entities import update_partner_linked_entities
from ..utils import (
    GetOrCreate,
    print_row,
    get_sheet_value,
)

from ..utils.excel_base_names import PATH_EXCEL_PARTNERS, PATH_EXCEL_PARTNERS_SHEET


class Command(BaseCommand):
    help = 'Загрузить базу контрагентов'

    def add_arguments(self, parser):
        parser.add_argument(
            '--drop-unexisted',
            action='store_true',
            help='удалять контрагентов, присутствующих в файле, но отсутствующих в базе',
        )

    def handle(self, *args, **options):
        need_drop = 'drop_unexisted' in options
        wb, cols, sheet, sheet_value = get_sheet_value(PATH_EXCEL_PARTNERS, PATH_EXCEL_PARTNERS_SHEET, 2)

        get_or_create = GetOrCreate()

        with transaction.atomic():
            country = get_or_create(m.Country, title='Республика Казахстан')
            db_rows = [
                populate_partner_row_from_db_values(
                    get_or_create, country, populate_partner_row_from_sheet(sheet_value, row_idx)
                )
                for row_idx in range(3, sheet.max_row + 1)
            ]

            handled_partners = {x.id: False for x in m.Partner.objects.all()}

            partners: T_GROUPS_OF_PARTNERS = group_partners_by_key(db_rows)
            bulk_partners = []
            db_partners_by_keys: Dict[
                T_GROUPS_OF_PARTNERS_KEY, Tuple[List[m.Partner], T_GROUPS_OF_PARTNERS_VALUES]
            ] = {}

            for key, partner_rows in partners.items():

                query_args = get_args_to_search_partners(partner_rows[0])

                if not query_args:
                    print('ERROR: empty partner')
                    print_row(partner_rows[0].r.row_idx, sheet)
                else:
                    it_q_partners = m.Partner.objects.filter(**query_args)
                    if not it_q_partners:
                        _args_to_create_partner = get_args_to_create_partner(partner_rows)
                        bulk_partners.append((m.Partner(**_args_to_create_partner), key, partner_rows))
                    else:
                        db_partners_by_keys[key] = (list(it_q_partners), partner_rows)
                        if len(it_q_partners) > 1:
                            print(f'ERROR: query of partners is non unique: {len(it_q_partners)}')
                            for partner in it_q_partners:
                                print('--', partner)

                        for partner in it_q_partners:
                            update_partner(handled_partners, partner, partner_rows[0])

            if bulk_partners:
                created = m.Partner.objects.bulk_create([partner for (partner, key, rows) in bulk_partners])
                for (created_partner, (key, rows)) in zip(
                    created, [(key, rows) for (partner, key, rows) in bulk_partners]
                ):
                    db_partners_by_keys[key] = ([created_partner], rows)

            all_partners = m.Partner.objects.all()
            all_partners_by_title: Dict[str, List[m.Partner]] = {}
            for _p in all_partners:
                try:
                    all_partners_by_title[_p.title].append(_p)
                except KeyError:
                    all_partners_by_title[_p.title] = [_p]

            for key, (partner_list, partner_rows) in db_partners_by_keys.items():
                for it_upd_partner in partner_list:
                    update_partner_linked_entities(
                        get_or_create, all_partners_by_title, it_upd_partner, partner_rows, need_drop
                    )

            if need_drop:
                m.Partner.objects.filter(
                    id__in=[k for (k, handled) in handled_partners.items() if not handled]
                ).delete()
