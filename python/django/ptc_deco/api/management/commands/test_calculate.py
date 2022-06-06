from django.core.management.base import BaseCommand
import ptc_deco.api.models as m
from loguru import logger

from ptc_deco.api.estimate.estimate import EstimateCalc
from ptc_deco.api.estimate.estimate_models_api import appendix_query


class Command(BaseCommand):
    help = 'Расчеты'

    def handle(self, *args, **options):
        '''выбрать все конструкции приложения, посчитать для каждой стоимость аренды,
         потом просуммировать по группе Город - формат -дата начала - дата окончания'''

        appendix_key = '202005123434'
        appendix_obj = appendix_query(code=appendix_key)[0]
        project_obj = appendix_obj.project
        logger.success(('project_code', project_obj.code, 'appendix_code', appendix_key))

        (EstimateCalc(project_obj, appendix_obj)
         .full_calc(**{'appendix__code': appendix_key})
         .test_print()
         )

        (EstimateCalc(project_obj)
         .full_calc()
         .test_print()
         )

        # group_reservation = Reservation.objects.filter(**appendices_kwargs).annotate(
        #     city=F('construction_side__construction__location__postcode__district__city'),
        #     format=F('construction_side__construction__format')
        # ).values(
        #     'format',
        #     'city',
        #     'date_to',
        #     'date_from',
        #     'id',
        #     # 'estimate__rent_price',
        #     # 'estimate__installation',
        #     # 'estimate__printing',
        #     'project__additional_costs__summa'
        # )
        # logger.success('=== Group Reservation ===')
        # for item in group_reservation:
        #     logger.info(item)












