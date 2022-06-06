from datetime import datetime

import pytz
from dateutil.relativedelta import relativedelta
from django.core.management import BaseCommand

from ptc_deco.api.management.commands.utils.send_mail import send
from ptc_deco.api.models import Contract, Appendix


class Command(BaseCommand):
    help = 'Сделать рассылку по почтам менеджеров и юристов по простроченім Договорам и Приложениям к ним'

    def handle(self, *args, **options):
        deadline = datetime.now(pytz.utc) - relativedelta(months=1)

        expired_contacts = Contract.objects.filter(created_at__lt=deadline, return_status=False)
        expired_appendixes = Appendix.objects.filter(created_date__lt=deadline, return_status=False)

        for con in expired_contacts:
            send(con)
        for app in expired_appendixes:
            send(app)
