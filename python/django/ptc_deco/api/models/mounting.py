from django.db import models
from django.utils import timezone

from .utils import named_meta, journal_save_delete
from .projects import Reservation, Design, Project
from .construction import ConstructionSide, Construction


from .utils import IMAGE_URL_MAX_LENGTH, EnumMountingRange


class MountingTask(models.Model):
    Meta = named_meta('Ремонтно-монтажная работа', 'MountingTask')

    title = models.CharField(max_length=1024, help_text='Наименование', null=True, unique=True)


@journal_save_delete
class Mounting(models.Model):
    Meta = named_meta('Монтаж', 'Mounting')

    start_mounting = models.DateTimeField(help_text='Дата монтажа', null=True)
    # Присваивается автоматически исходя из окончания бронирования
    end_mounting = models.DateTimeField(help_text='Дата демонтажа', null=True)

    mounting_done = models.BooleanField(help_text='Монтаж выполнен', null=False, default=False, blank=True)
    unmounting_done = models.BooleanField(help_text='Демонтаж выполнен', null=False, default=False, blank=True)

    comment = models.CharField(max_length=1024, help_text='Комментарий', null=True)

    downloaded_early = models.DateTimeField(null=True, help_text='Выгружено ранее')

    archived = models.BooleanField(help_text='В архиве', null=False, default=False, blank=True)

    RANGE_CHOICES = [
        (EnumMountingRange.HIGEST.value, 'Внеочередной'),
        (EnumMountingRange.HIGH.value,   'Высокий'),
        (EnumMountingRange.MEDIUM.value, 'Средний'),
        (EnumMountingRange.LOW.value,    'Низкий'),
        (EnumMountingRange.LOWEST.value, 'Самый низкий'),
    ]

    mounting_range = models.IntegerField(help_text='Уровень приоритетности', choices=RANGE_CHOICES, default=0)

    mounting_task = models.ForeignKey(
        MountingTask,
        help_text='Монтажи -> Ремонтно-монтажная работа',
        related_name='mountings',
        null=True,
        on_delete=models.DO_NOTHING
    )

    reservation = models.ForeignKey(
        Reservation,
        help_text='Монтажи -> Бронирование',
        related_name='reservation_mountings',
        null=True,
        on_delete=models.CASCADE
    )

    construction_side = models.ForeignKey(
        ConstructionSide,
        help_text='Монтажи -> стороны конструкций',
        related_name='mountings',
        null=True,
        on_delete=models.CASCADE
    )

    construction = models.ForeignKey(
        Construction,
        help_text='Монтажи -> конструкци',
        related_name='mountings',
        null=True,
        on_delete=models.CASCADE
    )

    design = models.ForeignKey(
        Design,
        help_text='Монтажи -> Монтируемый Дизайн',
        related_name='design_mountings',
        null=True,
        on_delete=models.DO_NOTHING
    )
    unmounting_design = models.ForeignKey(
        Design,
        help_text='Монтажи -> Демонтируемый Дизайн',
        related_name='design_unmountings',
        null=True,
        on_delete=models.DO_NOTHING
    )
    previous_design = models.ForeignKey(
        Design,
        help_text='Предыдущий дизайн',
        related_name='previous_design_mountings',
        null=True,
        on_delete=models.DO_NOTHING
    )

    # Параметры заполнения информации об экипаже в базе – номер экипажа, имя человека и город
    crew = models.ForeignKey(
        'Crew',
        help_text='Монтажи -> Экипаж',
        related_name='mountings',
        null=True,
        on_delete=models.SET_NULL
    )

    def save(self, *args, **kwargs):
        if self.id:
            self.previous_design = Mounting.objects.get(id=self.id).design

        return super().save(*args, **kwargs)


class AdvertPromoCompany(models.Model):
    Meta = named_meta('Рекламная промо-компания', 'AdvertPromoCompany')

    start = models.DateTimeField(help_text='Дата начала', null=True)
    title = models.CharField(max_length=512, help_text='Название промо-компании', null=True)
    city = models.ForeignKey(
        'City',
        help_text='Рекламная компания -> Город',
        related_name='advert_promo_companies',
        null=True,
        on_delete=models.SET_NULL
    )
    project = models.ForeignKey(
        Project,
        help_text='Рекламная компания -> Проект',
        related_name='advert_promo_companies',
        null=False,
        on_delete=models.CASCADE
    )


class MountingPhoto(models.Model):
    Meta = named_meta('Фотоотчет о монтаже', 'MountingPhoto')

    # TODO: вынести в отдельную таблицу
    photo = models.ImageField(upload_to='mounting', help_text='Фотоотчет о монтаже', null=True,
                              max_length=IMAGE_URL_MAX_LENGTH)
    date = models.DateTimeField(help_text='Дата фотоотчета о монтаже', null=True)
    num = models.CharField(max_length=128, help_text='Номер фотографии о монтаже', null=True)

    mounting = models.ForeignKey(
        Mounting,
        help_text='Монтажная работа -> фотоотчет о монтаже',
        related_name='photos',
        null=False,
        on_delete=models.CASCADE
    )

