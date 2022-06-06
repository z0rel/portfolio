import openpyxl
import os
from os.path import join
from collections import namedtuple
from copy import deepcopy
from django.db import transaction

from django.core.management.base import BaseCommand, CommandError
from .load_self_company_info import load_self_company_info

# fmt: off

class Command(BaseCommand):
    help = 'Загрузить константы в базу данных'

    def handle(self, *args, **options):
        from ptc_deco.api import models as m

        with transaction.atomic():
            m.TechProblems.objects.bulk_create(
                [
                    m.TechProblems(title='Разбита створка'),
                    m.TechProblems(title='Разбито стекло'),
                    m.TechProblems(title='Нет напряжения'),
                    m.TechProblems(title='Не работает часть диодов'),
                    m.TechProblems(title='Не работают все диоды'),
                    m.TechProblems(title='Неисправность амортизатора'),
                    m.TechProblems(title='Не работает двигатель'),
                    m.TechProblems(title='Панель (вариаторы)'),
                    m.TechProblems(title='Неисправность постеродержателя'),
                    m.TechProblems(title='Неисправность лавочки'),
                    m.TechProblems(title='ДТП'),
                ]
            )
            m.MountingTask.objects.bulk_create(
                [
                    m.MountingTask(title='Замена створки'),
                    m.MountingTask(title='Замена стекла'),
                    m.MountingTask(title='Подключение'),
                    m.MountingTask(title='Замена диодов'),
                    m.MountingTask(title='Замена двигателя'),
                    m.MountingTask(title='Замена амортизатора'),
                    m.MountingTask(title='Ремонт / замена панели(вариатора)'),
                    m.MountingTask(title='Ремонт постеродержателя'),
                    m.MountingTask(title='Замена лавочки'),
                    m.MountingTask(title='ДТП'),
                    m.MountingTask(title='Помывка'),
                    m.MountingTask(title='Помеха'),
                    m.MountingTask(title='Дополнительный ночной фотоотчет'),
                    m.MountingTask(title='Дополнительный дневной фотоотчет'),
                ]
            )
            m.WorkingSector.objects.bulk_create(
                [
                    m.WorkingSector(title='Alcohol (Beers)', description='Алкоголь (пиво)'),
                    m.WorkingSector(title='Alcohol (non-beers)', description='Алкоголь (кроме пива)'),
                    m.WorkingSector(title='Electrical Appliances', description='Электроприборы'),
                    m.WorkingSector(title='Entertainment, Leisure, Media', description='Развлечения, Досуг, Медиа'),
                    m.WorkingSector(title='Fashion, Fashion Retailers', description='Мода, Модные товары'),
                    m.WorkingSector(title='Financial', description='Финансовый сектор'),
                    m.WorkingSector(title='Government', description='Правительство'),
                    m.WorkingSector(title='Grocery, Food', description='Бакалея, Продукты питания'),
                    m.WorkingSector(title='Holiday, Travel, Transport', description='Отдых, Путешествия, Транспорт'),
                    m.WorkingSector(title='Online Retailers', description='Интернет-магазины'),
                    m.WorkingSector(title='Online Platforms', description='Интернет Платформы'),
                    m.WorkingSector(title='Luxury Goods', description='Предметы роскоши'),
                    m.WorkingSector(title='Motors', description='Автомобили'),
                    m.WorkingSector(title='Non Fashion Retailers', description='Товары общего потребления'),
                    m.WorkingSector(title='Other', description='Другой'),
                    m.WorkingSector(title='Personal Care', description='Личная гигиена'),
                    m.WorkingSector(title='Restaurants', description='Ресторан'),
                    m.WorkingSector(title='Telecoms Equipment & Computing', description='Телекоммуникационное оборудование и компьютеры'),
                    m.WorkingSector(title='Tobacco', description='Табак'),
                    m.WorkingSector(title='Toys & Games', description='Игрушки и игры'),
                    m.WorkingSector(title='Utilities', description='Коммунальные услуги'),
                ]
            )
            m.PartnerType.objects.bulk_create(
                [
                    m.PartnerType(title='Клиент'),
                    m.PartnerType(title='Рекламодатель'),
                    m.PartnerType(title='Рекламное агентство'),
                    m.PartnerType(title='Поставщик'),
                ]
            )
            m.ClientType.objects.bulk_create(
                [
                    m.ClientType(title='Новый', description='Трудовой'),
                    m.ClientType(title='Новый', description='По личным связям'),
                    m.ClientType(title='Новый', description='Корпоративный'),
                    m.ClientType(title='Текущий', description='Трудовой'),
                    m.ClientType(title='Текущий', description='По личным связям'),
                    m.ClientType(title='Текущий', description='Корпоративный'),
                    m.ClientType(title='Текущий', description='Исторический'),
                    m.ClientType(title='Текущий', description='Переданный'),
                ]
            )
            m.ContractType.objects.bulk_create(
                [m.ContractType(name='с клиентом'), m.ContractType(name='с поставщиком')]
            )
            m.ReservationType.objects.bulk_create(
                [
                    m.ReservationType(title='Свободно', level=0, ikey=m.ReservationType.ReservationTypeInteger.FREE),
                    m.ReservationType(title='Забронировано', level=1, ikey=m.ReservationType.ReservationTypeInteger.RESERVED),
                    m.ReservationType(title='Отменено', level=0, ikey=m.ReservationType.ReservationTypeInteger.CANCELLED),
                    m.ReservationType(title='Утверждено', level=2, ikey=m.ReservationType.ReservationTypeInteger.SETTED),
                    m.ReservationType(title='Продано', level=3, ikey=m.ReservationType.ReservationTypeInteger.SALED),
                    m.ReservationType(title='Недоступно', level=4, ikey=m.ReservationType.ReservationTypeInteger.UNAVAILABLE),
                ]
            )
            m.Obstruction.objects.bulk_create(
                [
                    m.Obstruction(title='Дерево'),
                    m.Obstruction(title='Столб'),
                    m.Obstruction(title='Сан. обрезка'),
                    m.Obstruction(title='Знак'),
                    m.Obstruction(title='Светофор'),
                    m.Obstruction(title='Урна'),
                    m.Obstruction(title='Конкурент'),
                    m.Obstruction(title='Другая конструкция'),
                ]
            )
            m.StaticAdditionalCosts.objects.bulk_create(
                [
                    m.StaticAdditionalCosts(category=m.StaticAdditionalCosts.CAT_MOUNTING, name='Монтаж'),
                    m.StaticAdditionalCosts(category=m.StaticAdditionalCosts.CAT_PRINTING, name='Печать'),
                    m.StaticAdditionalCosts(category=m.StaticAdditionalCosts.CAT_NALOG, name='Налог'),
                    m.StaticAdditionalCosts(category=m.StaticAdditionalCosts.CAT_DISCOUNT_NALOG, name='Скидка на налог'),
                    m.StaticAdditionalCosts(category=m.StaticAdditionalCosts.CAT_SKETCHES, name='Согласование эскизов'),
                    m.StaticAdditionalCosts(category=m.StaticAdditionalCosts.CAT_BRANDING, name='Оформление брендированных конструкций'),
                    m.StaticAdditionalCosts(category=m.StaticAdditionalCosts.CAT_ADDITIONAL_MOUNTING, name='Дополнительный монтаж'),
                    m.StaticAdditionalCosts(category=m.StaticAdditionalCosts.CAT_ADDITIONAL_PHOTOSET, name='Дополнительный фотоотчет'),
                    m.StaticAdditionalCosts(category=m.StaticAdditionalCosts.CAT_ADDITIONAL_PRINTING, name='Дополнительная печать'),
                ]
            )
            m_country = m.Country.objects.create(title='Республика Казахстан')
            m.City.objects.create(title='Алматы', country=m_country)
            m.City.objects.create(title='Нур-Султан', country=m_country)
            load_self_company_info()
# fmt: on
