import sys

from django.db import transaction
from django.core.management.base import BaseCommand
from django.contrib.auth.models import AbstractUser, Group

from .utils import read_yaml
from ptc_deco.api.models import CustomUser, EmployeePosition


class Command(BaseCommand):
    help = 'Загрузить данные Пользователя, Группы, Должности в базу данных'

    @transaction.atomic
    def handle(self, *args, **options):
        try:
            data = read_yaml('custom_user.yml')
        except FileNotFoundError as e:
            print(e)
            sys.exit()
        else:
            for value in data['group']['name']:
                Group.objects.get_or_create(name=value)
            for value in data['employee_position']['title']:
                EmployeePosition.objects.get_or_create(title=value)
            for _, values in data['custom_user'].items():
                values['employee_position'] = EmployeePosition.objects.get(title=values['employee_position'])

                group = Group.objects.get(name=values.pop('groups'))
                custom_user = CustomUser.objects.get_or_create(**values)[0]
                custom_user.groups.add(group)
