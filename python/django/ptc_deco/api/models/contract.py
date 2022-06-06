from django.db import models
from ...logger.middleware import LoggedInUser

from .utils import named_meta, journal_save_delete, IMAGE_URL_MAX_LENGTH
from .users import CustomUser
from .projects import Project


@journal_save_delete
class ContractType(models.Model):
    Meta = named_meta('Тип договора', 'ContractType')

    name = models.CharField(max_length=128, help_text='Тип договора', null=True, unique=True)


@journal_save_delete
class Contract(models.Model):
    Meta = named_meta('Договор', 'Contract')

    code = models.CharField(max_length=128, help_text='Код', null=True, unique=True)
    serial_number = models.CharField(max_length=128, help_text='Порядковый номер договора', null=True)
    registration_date = models.DateTimeField(help_text='Дата заключения', null=True)
    start = models.DateTimeField(help_text='Дата начала действия', null=True)
    end = models.DateTimeField(help_text='Дата окончания действия', null=True)

    payment_date = models.DateTimeField(help_text='Срок оплаты', null=True)

    signatory_position = models.CharField(max_length=256, help_text='Должность подписанта', null=True)
    signatory_one = models.CharField(max_length=256, help_text='Подписант в именительном падеже', null=True)
    signatory_two = models.CharField(max_length=256, help_text='Подписант в родительном падеже', null=True)
    based_on_document = models.CharField(
        max_length=256,
        help_text='Документ, на основании которого действует подписант',
        null=True,
    )
    return_status = models.BooleanField(default=True, help_text='Статус возврата', null=False)
    contract_pdf = models.ImageField(upload_to='contract', help_text='Договор', null=True,
                                     max_length=IMAGE_URL_MAX_LENGTH)

    additionally_agreement_pdf = models.ImageField(upload_to='contract', help_text='Дополнительное соглашение', null=True,
                                                max_length=IMAGE_URL_MAX_LENGTH)
    comment = models.TextField(help_text='Комментарий', null=True)
    created_at = models.DateTimeField(auto_now_add=True, help_text='Дата создания', null=True)
    updated_at = models.DateTimeField(auto_now=True, help_text='Дата обновления', null=True)

    is_archive = models.BooleanField(default=False, help_text='В архиве')

    contract_type = models.ForeignKey(
        ContractType,
        help_text='Договора -> Тип договора',
        related_name='contracts',
        null=True,
        on_delete=models.SET_NULL
    )
    partner = models.ForeignKey(
        'Partner',
        help_text='Договора -> Контрагент',
        related_name='contracts',
        null=True,
        on_delete=models.SET_NULL
    )
    creator = models.ForeignKey(
        CustomUser,
        help_text='Договора -> Создатель (кто внес данные)',
        related_name='created_contracts',
        null=True,
        on_delete=models.SET_NULL
    )
    initiator = models.ForeignKey(
        CustomUser,
        help_text='Договора -> Инициатор',
        related_name='initiated_contracts',
        null=True,
        on_delete=models.SET_NULL
    )
    sales_manager = models.ForeignKey(
        CustomUser,
        help_text='Договора -> Менеджер по продажам',
        related_name='sales_manager_on_contracts',
        null=True,
        on_delete=models.SET_NULL
    )

    def save(self, *args, **kwargs):
        logged_in = LoggedInUser()

        if logged_in.current_user:
            self.creator = logged_in.current_user

        return super(Contract, self).save(*args, **kwargs)

    def __str__(self):
        return f"Договор №0000{ self.id }"


@journal_save_delete
class LastAppendixNumber(models.Model):
    Meta = named_meta('Вспомогательная таблица максимальных номеров приложений к договору', 'LastAppendixNumber',
                      unique_together=['year', 'month'])

    year = models.IntegerField(help_text='Год')
    month = models.IntegerField(help_text='Месяц')
    max_number = models.IntegerField(help_text='Максимальный номер', null=True)


@journal_save_delete
class Appendix(models.Model):
    Meta = named_meta('Приложение к договору', 'Appendix')
    #  Генерируется автоиатичемки ГГГГММ{порядковый номер}
    code = models.CharField(max_length=128, help_text='Номер приложения', null=True)
    num_in_month = models.IntegerField(help_text='Номер приложения в месяце', null=True)
    created_date = models.DateTimeField(help_text='Дата создания приложения', null=True)

    period_start_date = models.DateTimeField(help_text='Период приложения - дата начала размещения', null=True)
    period_end_date = models.DateTimeField(help_text='Период приложения - дата окончания размещения', null=True)

    return_status = models.BooleanField(default=True, help_text='Статус возврата', null=False)
    additionally_agreement = models.ImageField(upload_to='appendix', help_text='Скан доп. соглашения', null=True,
                                               max_length=IMAGE_URL_MAX_LENGTH)

    # TODO в тех. задании написано что информация об обновлении приложения не должна сохраняться
    updated_at = models.DateTimeField(auto_now=True, help_text='Дата обновления', null=True)

    signatory_one = models.CharField(max_length=256, help_text='Подписант в именительном падеже', null=True)
    signatory_two = models.CharField(max_length=256, help_text='Подписант в родительном падеже', null=True)
    payment_date = models.DateTimeField(help_text='Срок оплаты', null=True)
    signatory_position = models.CharField(max_length=256, help_text='Должность подписанта', null=True)

    is_archive = models.BooleanField(default=False, help_text='В архиве')

    contract = models.ForeignKey(
        Contract,
        help_text='Приложения к договору -> Договор',
        related_name='contract_appendices',
        null=True,
        on_delete=models.SET_NULL
    )
    project = models.ForeignKey(
        Project,
        help_text='Приложения к договору -> Проект',
        related_name='project_appendices',
        null=True,
        on_delete=models.DO_NOTHING
    )
    creator = models.ForeignKey(
        CustomUser,
        help_text='Приложения к договору -> Создатель (кто внес данные)',
        related_name='created_contracts_appendices',
        null=True,
        on_delete=models.SET_NULL
    )

    sales_manager = models.ForeignKey(
        CustomUser,
        help_text='Приложения к договору -> Менеджер по продажам',
        related_name='sales_manager_on_contracts_appendices',
        null=True,
        on_delete=models.SET_NULL
    )

    def save(self, *args, **kwargs):
        if self.created_date and self.num_in_month is None:
            num: LastAppendixNumber = LastAppendixNumber.objects.get_or_create(year=self.created_date.year,
                                                                               month=self.created_date.month)[0]
            num.max_number = 1 if num.max_number is None else num.max_number + 1
            num.save()
            self.num_in_month = num.max_number
            if self.code is None:
                self.code = f'{self.created_date.year}-{str(self.created_date.month).zfill(2)}-{str(self.num_in_month).zfill(5)}'

        logged_in = LoggedInUser()

        if logged_in.current_user:
            self.creator = logged_in.current_user

        return super(Appendix, self).save(*args, **kwargs)


    def __str__(self):
        return f'Приложение к "{self.contract.__str__()}"'


class LastSerialNumberContract(models.Model):
    __metaclass__ = named_meta('Последний порядковый номер договора', 'Последний порядковый номер договора')

    year = models.SmallIntegerField(help_text='Год начала действия договора', null=False, unique=True)
    number = models.BigIntegerField(null=True, help_text='Последний порядковый номер договора')


class SelfCompanyInfo(models.Model):
    __metaclass__ = named_meta('Информация об организации-Исполнителе', 'Информация об организации-Исполнителе')

    company_name = models.CharField(max_length=512, help_text='Название организации', null=True)
    attachment_city = models.CharField(max_length=512, help_text='Город в документе-приложении', null=True)
    signatory_brief_one = models.CharField(max_length=512, help_text='Подписант в именительном падеже (с инициалами)', null=True)
    signatory_brief_two = models.CharField(max_length=512, help_text='Подписант в родительном падеже (с инициалами)', null=True)
    signatory_one = models.CharField(max_length=512, help_text='Подписант в именительном падеже (ФИО полностью)', null=True)
    signatory_two = models.CharField(max_length=512, help_text='Подписант в родительном падеже (ФИО полностью)', null=True)
    signatory_position_one = models.CharField(max_length=512, help_text='Должность подписанта в именительном падеже', null=True)
    signatory_position_two = models.CharField(max_length=512, help_text='Должность подписанта в родительном падеже', null=True)
    document_base_one = models.CharField(max_length=512, help_text='Документ-основание в именительном падеже', null=True)
    document_base_two = models.CharField(max_length=512, help_text='Документ-основание в родительном падеже', null=True)
