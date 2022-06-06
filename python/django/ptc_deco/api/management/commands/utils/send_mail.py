import logging

from django.core.mail import send_mail

from ptc_deco.api.models import CustomUser, Contract
from ptc_deco.main import settings

logger = logging.getLogger(__name__)


def get_lawyers_mail():
    lawyers = CustomUser.objects.filter(groups__name="Юрист")
    return [l.email for l in lawyers]


def send(obj):
    if isinstance(obj, Contract):
        subject = f'Просроченый Договор'
        message = f'Договор №{obj.serial_number} не вернулся спустя месяц после окончания периода размещения'
    else:
        subject = f'Просрочено Приложение к договору'
        message = f'Приложение к договору №{obj.code} не вернулось спустя месяц после окончания периода размещения'

    from_email = settings.EMAIL_HOST_USER
    to_emails = get_lawyers_mail() + [obj.sales_manager.email]
    send_mail(subject, message, from_email, to_emails)
    logger.info(
        f'Send mail about {"contract" if isinstance(obj, Contract) else "appendix"} with code '
        f'{obj.serial_number if isinstance(obj, Contract) else obj.code} to emails: {to_emails}')
