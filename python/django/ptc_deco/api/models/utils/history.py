from django.db import models
from django.contrib import admin
from django.conf import settings
from django_currentuser.middleware import get_current_authenticated_user

from ...models.users import CustomUser


def create_model(name, fields=None, app_label='', module='', options=None, admin_opts=None):
    """
    Create specified model
    """
    class Meta:
        # Using type('Meta', ...) gives a dictproxy error during model creation
        pass

    if app_label:
        # app_label must be set using the Meta inner class
        setattr(Meta, 'app_label', app_label)

    # Update Meta with any options that were provided
    if options is not None:
        for key, value in options.items():
            setattr(Meta, key, value)

    # Set up a dictionary to simulate declarations within a class
    attrs = {'__module__': module, 'Meta': Meta}

    # Add in any fields that were provided
    if fields:
        attrs.update(fields)

    # Create the class, which automatically triggers ModelBase processing
    model = type(name, (models.Model,), attrs)

    # Create an Admin class if admin options were provided
    if admin_opts is not None:
        class Admin(admin.ModelAdmin):
            pass
        for key, value in admin_opts:
            setattr(Admin, key, value)
        admin.site.register(model, Admin)

    return model


def create_history_model(name, app_label, module, target_model: models.Model):
    fields = {
        'target': models.ForeignKey(target_model, null=True, on_delete=models.SET_NULL),
        'protected_target_id': models.IntegerField(default=0),
        'manipulation_type': models.CharField(max_length=55, null=False, blank=True, default=''),
        'field_type': models.CharField(max_length=255, null=False, blank=True),
        'previous_state': models.JSONField(max_length=512, null=True, blank=True),
        'new_state': models.JSONField(max_length=512, null=True, blank=True),
        'created': models.DateTimeField(auto_now_add=True, null=False),
        'user': models.ForeignKey(CustomUser, null=True, on_delete=models.SET_NULL)
    }
    model = create_model(name, fields, app_label=app_label, module=module)
    return model


class HistoryModel:
    def __init__(self, name, app_label, module, target_model: models.Model):
        self.model = create_history_model(name, app_label, module, target_model)
        self.user = get_current_authenticated_user()
        self.enabled = settings.RECORD_HISTORY

    def save_diff(self, current, previous=None):
        if not self.enabled:
            return
        if not previous:
            self.start(current)

        for field in current._meta.get_fields():
            try:
                if getattr(current, field.name) != getattr(previous, field.name):
                    record = self.model()
                    record.target = current
                    record.protected_target_id = current.id
                    record.manipulation_type = field.name
                    record.field_type = field.__class__.__name__
                    record.previous_state = str(getattr(previous, field.name))
                    record.new_state = str(getattr(current, field.name))
                    if self.user:
                        record.user = self.user
                    record.save()
            except AttributeError:
                continue

    def start(self, current):
        if not self.enabled:
            return
        record = self.model()
        record.manipulation_type = 'Создание записи'
        record.target = current
        record.protected_target_id = current.id
        if self.user:
            record.user = self.user
        record.save()

    def close(self, current):
        if not self.enabled:
            return
        record = self.model()
        record.manipulation_type = 'Удаление записи'
        record.protected_target_id = current.id
        if self.user:
            record.user = self.user
        record.save()
