from django.db import models
from .utils import named_meta
from . import CustomUser


class Notification(models.Model):
    Meta = named_meta('Уведомление', 'Notification')
    user = models.ForeignKey(CustomUser, on_delete=models.CASCADE, null=True, help_text='Пользователь')
    topic = models.CharField(max_length=255, default="Создан новый объект.", help_text='Тема')
    read = models.BooleanField(default=False, help_text='Прочитано')
    created_at = models.DateTimeField(auto_now_add=True, help_text='Дата создания', null=True)
    updated_at = models.DateTimeField(auto_now=True, help_text='Дата редактирования', null=True)


class ConstructionNotification(Notification):
    construction = models.ForeignKey('Construction', on_delete=models.CASCADE, null=True, help_text='Конструкция')

    def save(self, *args, **kwargs):
        self.topic = "Создана новая конструкция."
        super(ConstructionNotification, self).save(*args, **kwargs)
