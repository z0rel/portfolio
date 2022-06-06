from django.core.management.base import BaseCommand

from ptc_deco.xlsx.moduls import (construction_report_xlsx, non_rts_report_xlsx,
                                  street_furniture_report_xlsx, tax_report_xlsx,
                                  free_side_report_xlsx, notice_for_filing_report_xlsx,
                                  france_report, sales_report_xlsx, estimate_report_xlsx,
                                  occupancy_report_xlsx, handbook_of_parties_report_xlsx)


class Command(BaseCommand):
    help = 'Загрузить базу земельных участков'

    def handle(self, *args, **options):
        if options['constructor']:
            construction_report_xlsx.download_file()
        if options['street_furniture']:
            street_furniture_report_xlsx.download_file()
        if options['tax_report']:
            tax_report_xlsx.download_file()
        if options['non_rts']:
            non_rts_report_xlsx.download_file()
        if options['test_body']:
            tax_report_xlsx.get_body_sheet()
        if options['free_side']:
            free_side_report_xlsx.download_file()
        if options['notice_for_filing']:
            notice_for_filing_report_xlsx.download_file()
        if options['france_report']:
            france_report.download_file()
        if options['sales_report']:
            sales_report_xlsx.download_file()
        if options['estimate_report']:
            estimate_report_xlsx.download_file()
        if options['occupancy_report']:
            occupancy_report_xlsx.download_file()
        if options['handbook_of_parties_report']:
            handbook_of_parties_report_xlsx.generate_file()

    def add_arguments(self, parser):
        parser.add_argument(
            '-c',
            '--constructor',
            action='store_true',
            default=False,
            help='Генерация отчета "Информация о бронированиях.xlsx"'
        )
        parser.add_argument(
            '-sf',
            '--street_furniture',
            action='store_true',
            default=False,
            help='Генерация отчета "УМ.xlsx"'
        )
        parser.add_argument(
            '-tr',
            '--tax_report',
            action='store_true',
            default=False,
            help='Генерация отчета "Отчет по налогам.xlsx"'
        )
        parser.add_argument(
            '-nr',
            '--non_rts',
            action='store_true',
            default=False,
            help='Генерация отчета "НОН РТС.xlsx"'
        )
        parser.add_argument(
            '-t',
            '--test_body',
            action='store_true',
            default=False,
            help='Формирование тела документа в "Отчет по налогам.xlsx"'
        )
        parser.add_argument(
            '-fs',
            '--free_side',
            action='store_true',
            default=False,
            help='Генерация отчета "Отчет по свободным рекламным сторонам.xlsx"'
        )
        parser.add_argument(
            '-nff',
            '--notice_for_filing',
            action='store_true',
            default=False,
            help='Генерация отчета "Уведомление(Для подачи).xlsx"'
        )
        parser.add_argument(
            '-fr',
            '--france_report',
            action='store_true',
            default=False,
            help='Генерация отчета "Французкий отчет.xlsx"'
        )
        parser.add_argument(
            '-sr',
            '--sales_report',
            action='store_true',
            default=False,
            help='Генерация отчета "Отчет по продажам.xlsx"'
        )
        parser.add_argument(
            '-er',
            '--estimate_report',
            action='store_true',
            default=False,
            help='Генерация отчета "Смета.xlsx"'
        )
        parser.add_argument(
            '-or',
            '--occupancy_report',
            action='store_true',
            default=False,
            help='Генерация отчета "Отчет по заполняемости.xlsx"'
        )
        parser.add_argument(
            '-hp',
            '--handbook_of_parties_report',
            action='store_true',
            default=False,
            help='Генерация отчета "Выгрузка справочника сторон.xlsx"'
        )
