import sys
import os
import random
from collections import namedtuple

import openpyxl
from openpyxl.worksheet.worksheet import Worksheet

from django.db import transaction
from django.core.management.base import BaseCommand
from django.db.models.query import QuerySet
from typing import Union, List
from .utils import POSTCODE_TO_DISTRICT_MAP, get_postcode, TrSideTypes

from ptc_deco.api import models as m

RowPackage = namedtuple(
    'RowPackage',
    [
        's_code',
        's_city',
        's_address_market',
        's_format',
        's_side',
        's_adv_side',
        's_package'
    ]
)


#  Необходимые модели и поля к ним
# City: s_sity(title)+, (country_id=1 - обязательное); get_or_create
# Package: s_package(title)+, City; get_or_create
# Format: s_format(title)+, ModelConstruction(необходимо брать из существующих); get_or_create
# Addresses: s_marketing_address(address)+; get_or_create
# Location: Addresses; get_or_create
# Side: Format, s_side(title)+; get_or_create
# AdvertisingSide: s_adv_side(title)+, Side; get_or_create
# Construction: Location; get_or_create
# ConstructionSide: AdvertisingSide, Package, Construction; get_or_create

class GetOrCreate:
    def __init__(self):
        self.cached = {}

    def get_or_create(self, _model, **kwargs):
        ok, key, value = self._get_cached_value(_model, '', kwargs)
        if ok:
            return value

        obj = _model.objects.filter(**kwargs)
        result = obj[0] if obj else _model.objects.create(**kwargs)
        self.cached[key] = result
        return result

    def get_qs_or_create(self, _model, **kwargs):
        ok, key, value = self._get_cached_value(_model, 'qs', kwargs)
        if ok:
            return value

        obj = _model.objects.filter(**kwargs)
        result = obj if obj else [_model.objects.create(**kwargs)]
        self.cached[key] = result
        return result

    def create(self, _model, **kwargs):
        ok, key, value = self._get_cached_value(_model, '', kwargs)
        if ok:
            return value

        result = _model.objects.create(**kwargs)
        self.cached[key] = result
        return result

    def _get_cached_value(self, _model, additional_key, kwargs):
        key = tuple((_model._meta.model_name, additional_key, *sorted(kwargs.items())))
        try:
            result = self.cached[key]
            return True, key, result
        except KeyError:
            return False, key, None


class FormingData:
    def __init__(self):
        self.cached_storage = GetOrCreate()

    def get_formats(self, title: str) -> Union[m.Format, QuerySet]:
        obj = m.Format.objects.filter(title=title)
        if obj:
            return obj
        else:
            print(f"Формат '{title}' не найден")
            return None

    def get_address(self, row, need_raise, country):
        # для адреса доступен также почтовый код из кода рекламной стороны
        if row.s_code:
            postcode_value = row.s_code.split('.')[0]
            address = m.Addresses.objects.filter(address=row.s_address_market, postcode__title=postcode_value)
        else:
            postcode_value = None
            address = m.Addresses.objects.filter(address=row.s_address_market)

        if address:
            address = address[0]
        else:
            if not postcode_value:
                _, _, postcode = get_postcode(self.cached_storage.get_or_create, m, country, row.s_city, None, None)
            else:
                district = POSTCODE_TO_DISTRICT_MAP.get(postcode_value, None)
                if district is None:
                    need_raise = True
                    print(f'Район для почтового кода {postcode_value} не задан')
                    return [False, need_raise, address]
                _, _, postcode = get_postcode(self.cached_storage.get_or_create, m, country, row.s_city, district,
                                              postcode_value)
            address = self.cached_storage.create(m.Addresses, address=row.s_address_market, postcode=postcode)

        return [True, need_raise, address]

    def parse_and_load(self, data: List[RowPackage]):
        need_raise = False
        country = self.cached_storage.get_or_create(m.Country, title='Республика Казахстан')
        for row in data:
            need_continue = False
            city = self.cached_storage.get_or_create(m.City, title=row.s_city)
            package = self.cached_storage.get_or_create(m.Package, title=row.s_package, city=city)

            format = self.get_formats(title=row.s_format)
            if format is None:
                need_raise = True
                need_continue = True

            ok, need_raise, address = self.get_address(row, need_raise, country)
            need_continue = need_continue or not ok

            check_adv_sides = m.AdvertisingSide.objects.filter(title=row.s_adv_side)
            if not check_adv_sides:
                need_continue = True
                print(f'Тип рекламной стороны {row.s_adv_side} не существует')

            check_sides = m.Side.objects.filter(title=row.s_side)
            if not check_sides:
                need_continue = True
                print(f'Тип стороны {row.s_side} не существует, тип рекламной стороны: {row.s_adv_side}')

            if need_continue:
                continue

            location = self.cached_storage.get_qs_or_create(m.Location, marketing_address=address,
                                                            postcode=address.postcode)

            # Найти все стороны в заданном местоположении, среди них выбрать стороны с заданным форматом,
            # стороной и рекламной стороной - и присвоить им пакеты
            construction_sides = m.ConstructionSide.objects.filter(
                advertising_side__title=row.s_adv_side,              # с заданным типом рекламной стороны
                advertising_side__side__title=row.s_side,            # с заданным типом стороны
                advertising_side__side__format__title=row.s_format,  # с заданным типом формата
                construction__location__in=location                  # c заданным местоположением
            ).select_related(
                'advertising_side',
                'advertising_side__side',
                'advertising_side__side__format',
                'construction__location'
            )
            construction_sides.update(package=package)

        if need_raise:
            raise Exception('Не хватает данных для консистентной загрузки базы пакетов')



def get_data_sheet(sheet: Worksheet) -> List[RowPackage]:
    data_sheet: List[RowPackage] = []

    for line in sheet.iter_rows(min_col=1, min_row=2, values_only=True):
        s_side = line[4]
        s_adv_side = line[5]
        side_code = s_side.split(' ')[-1]
        adv_side_code = s_adv_side.split(' ')[-1]
        if (side_code, adv_side_code) in TrSideTypes.SET_TO_SWAP:
            s_side, s_adv_side = s_adv_side, s_side
            side_code, adv_side_code = adv_side_code, side_code

        row = RowPackage(
            s_code=line[0],
            s_city=line[1],
            s_address_market=line[2],
            s_format=TrSideTypes.tr_format(line[3]),
            s_side=TrSideTypes.tr_side_type(s_side, adv_side_code),
            s_adv_side=TrSideTypes.tr_adv_side_type(s_adv_side),
            s_package=line[6])
        data_sheet.append(row)
    return data_sheet


class Command(BaseCommand):
    @transaction.atomic
    def handle(self, *args, **options):
        try:
            wb = openpyxl.load_workbook(os.path.join('.', 'datasource', 'Пример пакетов.xlsx'))
        except FileNotFoundError as e:
            print(e)
            sys.exit()
        else:
            sheet = wb.get_sheet_by_name('Лист1')
            sheet_value = get_data_sheet(sheet)
            forming_data = FormingData()
            forming_data.parse_and_load(sheet_value)
            wb.close()
