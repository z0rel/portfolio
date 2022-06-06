from django.contrib.auth.models import Permission, ContentType
from django.core.management.base import BaseCommand


custom_permissions = {
    'delete_estimate_item': 'Может удалять элементы сметы',
    'edit_estimate_item': 'Может изменять значения полей строк сметы',
    'add_estimate_item_to_appendix': 'Может добавлять строки из сметы проекта в смету приложения',
    'batch_add_construction_sides_to_package': 'Может добавлять пакетно стороны конструкции в пакет',
    'create_mounting_task_for_project': 'Может создавать задание на монтирование для проекта',
    'generate_appendix_docx': 'Может генерировать приложения к Договору',
    'download_packages_info': 'Может загружать информацию по пакетам',
    'update_package_reservation_type': 'Может изменять тип брони пакета',
    'upload_packages': 'Может выгружать информацию о пакетном размещении',
}


class Command(BaseCommand):
    help = 'Load custom permissions.'

    def handle(self, *args, **options):
        content_type = ContentType.objects.get_or_create(app_label='api', model='')

        permissions = [
            Permission(name=name, codename=codename, content_type=content_type)
            for codename, name in custom_permissions.items()
        ]

        Permission.objects.bulk_create(permissions)


