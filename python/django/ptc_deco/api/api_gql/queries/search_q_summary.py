# from graphene.relay import Node, ConnectionField
# from graphene import Connection, Int
# from graphene_django import DjangoObjectType
# import graphene as g
# from django.db.models import F
# from django_print_sql import print_sql
# import sqlparse

# == Сводка ==
# Сводка выводится при выборе месяца.
# Менеджер указывает период, который есть в проекте, и ему выдается табличка, содержащая колонки:

# Период
# Номер_проекта
# Контрагент
# Бренд
# Город
# Формат
# Количество
# Монтаж
# Фотоотчет_о_монтаже
# Фотоотчет_доп._день
# Фотоотчет_доп._ночь
# Смета
# Приложение
# Счет
# АВР
#
# Строчки объединяются по принципу город – конструкция.
#
#
# в проекте есть несколько форматов, которые есть в нескольких городах,
#   - в сводке строчки объединяются по всем адресам каждого города
#   - выводятся все конструкции поформатно в одном городе и в другом городе
#
# Пользователь может просматривать общую сводку по всем проектам, которые были созданы им или другими пользователями,
# за указанный период. Для этого ему необходимо зайти во вкладку поиска проектов и нажать на кнопку сводка проектов.
#
# TODO: фотоотчеты, сметы - могут быть заполнены пользователем сомостоятельно.
#
# TODO: За 3 дня до предполагаемого размещения клиента, если в проекте есть бронь со статусом продано и утверждено,
# TODO: пользователю отправляется уведомление о том, что проект не распределен на монтаж.
# TODO: После 5 дней после старта размещения пользователю приходит уведомление об отсутствии фотоотчета,
# TODO: если пользователь не указал обратное.
# TODO: 25 числа каждого месяца, если по проекту было размещение, пользователю отправляется уведомление,
# TODO: если не хватает приложения, счета, АВР.
#


from ...models import Format, City, Brand, Partner, Project, Appendix


# def grapheneField(modelClass):
#     classname = f'V{modelClass.__name__}Type'
#     return g.Field(
#         type(
#             classname,
#             (DjangoObjectType,),
#             {'Meta': type(f'{classname}.Meta', (), {'model': modelClass, 'interfaces': (Node,)})},
#         )
#     )
#
#
# class SummaryType(g.ObjectType):
#     class Meta:
#         interfaces = (Node,)
#
#     date_start = g.Date()  # Период TODO - начало чего - проекта, монтажа еще чего-то
#     date_end = g.Date()  # Период TODO - начало чего - проекта, монтажа еще чего-то
#     project = g.String()  # Проект
#     partner = g.String()  # Контрагент - client
#     brand = g.String()  # Бренд
#     city = g.String()  # Город
#     format = g.String()  # Формат
#     count = g.Int()  # количество - чего?
#     installation = g.Int()  # TODO: Монтаж - монтажей в модели нет
#     installation_photo_day = g.Int()  # TODO: Фотоотчет_доп._день
#     installation_photo_night = g.Int()  # TODO: Фотоотчет_доп._ночь
#     estimatePrice = g.Decimal()  # Смета
#     additionalPrice = g.Decimal()  # Приложение
#     invoicePrice = g.Decimal()  # Счет
#     abp = g.Boolean()  # TODO АВР  что это такое?
#
#
# class SummaryConnection(Connection):
#     class Meta:
#         node = SummaryType
#
#     count = Int()
#
#     def resolve_count(root, info):
#         return len(root.edges)
#
#
# class SummaryQuery(g.ObjectType):
#     # summary = g.List(SummaryType, date_start=g.Date(), date_end=g.Date())  # TODO:, question_id=graphene.String()) - задать аргументы запроса
#     summary = ConnectionField(
#         SummaryConnection, date_start=g.Date(), date_end=g.Date()
#     )  # TODO:, question_id=graphene.String()) - задать аргументы запроса
#
#     @staticmethod
#     def resolve_summary(parent, date_start=None, date_end=None):
#         # with print_sql(count_only=False):
#         if 1:
#             query = Appendix.objects.filter(  # всё что в filter - становится inner join
#                 # invoice__project=F('project'),
#                 # reservation__project=F('project'), # бронирование - left outer join
#             ).annotate(
#                 date_start=F('reservation__date_from'),
#                 date_end=F('reservation__date_to'),
#                 project_title=F('project__title'),
#                 partner_title=F('project__client__title'),
#                 brand_title=F('project__brand__title'),
#                 city=F('reservation__construction_side__construction__city__title'),
#                 format=F('reservation__construction_side__format__title'),
#                 estimated_price=F('reservation__estimate__value_agent_commission'),
#                 invoice_whole_sum=F('invoice__whole_sum'),
#             )
#             try:
#                 print(sqlparse.format(sqlparse.split(str(query.query))[0], reindent=True, keyword_case='upper'))
#             except Exception as e:
#                 print('Error', str(e))
#
#             # print(str(query.query))
#             # TODO: need aggregate data by construction ? may be Case when then
#
#             # TODO:
#             # if date_start:
#             #     query = query.filter(reservation__date_from__gte=date_start)
#             # if date_end:
#             #     query = query.filter(reservation__date_to__lt=date_end)
#             return []
#
#             dst = []
#             for q in query.iterator():
#                 dst.append(
#                     SummaryType(
#                         date_start=q.date_start,
#                         date_end=q.date_end,
#                         project=q.project_title,  # +
#                         partner=q.partner_title,  # +
#                         brand=q.brand_title,  # +
#                         city=q.city,  # +
#                         format=q.format,  # +
#                         count=-1,
#                         installation=-1,  # -
#                         installation_photo_night=-1,  # -
#                         estimatePrice=q.estimated_price,  # TODO: какое число выводить из сметы в сводку
#                         additionalPrice=-1.0,  # +- ? TODO: в приложении нет цены
#                         invoicePrice=q.invoice_whole_sum,  # TODO: с НДС ? без НДС
#                         abp=False,  # >
#                     )
#                 )
#             return dst
#