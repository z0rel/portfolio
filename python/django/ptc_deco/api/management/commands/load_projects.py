import sys
from django.db import transaction
from django.core.management.base import BaseCommand
import iso8601

from .utils import read_yaml
from ptc_deco.api import models as m
from django.utils.timezone import make_aware


class Command(BaseCommand):
    help = 'Загрузить данные Проекты в базу данных'


    @transaction.atomic
    def handle(self, *args, **options):
        try:
            data = read_yaml('projects.yml')
            designs_yml = read_yaml('designs.yml')
        except FileNotFoundError as e:
            print(e)
            return

        self._cleanup()
        self._load(data, designs_yml)

    @transaction.atomic
    def _cleanup(self):
        m.Reservation.objects.all().delete()
        m.Project.objects.all().delete()
        m.AdvertPromoCompany.objects.all().delete()
        m.Appendix.objects.all().delete()
        m.Invoice.objects.all().delete()
        m.ProjectCities.objects.all().delete()
        m.Mounting.objects.all().delete()

    @transaction.atomic
    def _load(self, data, designs_yml):
        for _, values in data.items():
            # date_time_obj = datetime.datetime.strptime(date_time_str, '%Y-%m-%d %H:%M:%S.%f')
            appendices = values.pop('appendices', {})
            reservations = values.pop('reservations', {})
            advert_promo_companies = values.pop('advert_promo_companies', [])
            new_values = {**values}
            new_values['created_at'] = iso8601.parse_date(values['created_at'])
            new_values['brand'] = m.Brand.objects.filter(title=values['brand'])[0]
            new_values['client'] = m.Partner.objects.get(title=values['client'])
            new_values['creator'] = m.CustomUser.objects.get(username=values['creator'])
            new_values['sales_manager'] = m.CustomUser.objects.get(username=values['sales_manager'])
            new_values['back_office_manager'] = m.CustomUser.objects.get(username=values['back_office_manager'])

            agency_commission_value = new_values.pop('agency_commission_value', None)
            agency_commission_percent = new_values.pop('agency_commission_percent', None)

            if agency_commission_value is not None or agency_commission_percent is not None:
                ak = m.AgencyCommission.objects.create(
                    to_rent=True,
                    to_nalog=True,
                    to_print=True,
                    to_mount=True,
                    to_additional=True,
                    to_nonrts=True,
                    value=agency_commission_value,
                    percent=agency_commission_percent
                )
                new_values['agency_commission'] = ak

            project = m.Project.objects.get_or_create(**new_values)[0]
            project.save(created_at_internal=iso8601.parse_date(values['created_at']))

            for advert_promo_company in advert_promo_companies:
                designs = advert_promo_company.pop('designs', [])
                city = advert_promo_company.pop('city', None)
                if city:
                    city = m.City.objects.get(title=city)
                advert_promo_company_obj = m.AdvertPromoCompany.objects.create(project_id=project.id,
                                                                               city=city,
                                                                               **advert_promo_company)
                for design in designs:
                    design_yml_obj = designs_yml[design]
                    m.Design.objects.create(**design_yml_obj, advert_promo_company_id=advert_promo_company_obj.id)

            reservation_date_field = ('date_from', 'date_to')
            created_reservations = {}
            for key, reservation in reservations.items():
                for field in reservation_date_field:
                    reservation[field] = make_aware(reservation[field])

                reservation['project'] = project
                reservation['reservation_type'] = m.ReservationType.objects.get(title=reservation['reservation_type'])
                reservation_obj = m.Reservation.objects.get_or_create(**reservation)[0]
                created_reservations[key] = reservation_obj

            for _, appendix in appendices.items():
                invoices = appendix.pop('invoices') if 'invoices' in appendix else {}
                date_field = ('created_date', 'period_start_date', 'period_end_date', 'payment_date')
                att_reservations_keys = appendix.pop('reservations') if 'reservations' in appendix else []
                for field in date_field:
                    appendix[field] = make_aware(appendix[field])
                # values['estimate'] = Estimate.objects.filter(title=values['estimate']).first()
                contract_obj = m.Contract.objects.filter(code=appendix['contract'])
                if contract_obj:
                    contract_obj = contract_obj[0]
                    appendix['contract'] = contract_obj
                else:
                    appendix['contract'] = None

                appendix['project'] = project
                appendix['creator'] = m.CustomUser.objects.get(username=appendix['creator'])
                appendix['sales_manager'] = m.CustomUser.objects.get(username=appendix['sales_manager'])

                appendix_obj = m.Appendix.objects.get_or_create(**appendix)[0]
                for key in att_reservations_keys:
                    if key in created_reservations:
                       r: m.Reservation = created_reservations[key]
                       r.appendix.add(appendix_obj)
                       r.save()

                for _, invoice in invoices.items():
                    invoice['sum_without_nds'] = invoice.pop('sum_wo_nds')
                    invoice['payment_last_date'] = invoice.pop('date')
                    invoice['customer_payment_method'] = invoice.pop('method')
                    invoice['project'] = project
                    invoice['contract'] = contract_obj
                    invoice['appendix'] = appendix_obj
                    invoice['payment_last_date'] = iso8601.parse_date(invoice['payment_last_date'])
                    fp = m.Partner.objects.filter(title=invoice['partner'])
                    if fp:
                        invoice['partner'] = fp[0]
                        m.Invoice.objects.get_or_create(**invoice)


