import unicodedata
from django.conf import settings
from django.contrib.auth import password_validation
from django.contrib.auth.hashers import check_password, make_password, is_password_usable
from django.db import models
from django.contrib.auth.models import (
    Group,
    PermissionsMixin,
    send_mail,
    UserManager,
    UnicodeUsernameValidator,
    timezone,
)
from django.utils.crypto import salted_hmac

from .utils import named_meta, journal_save_delete, DECIMAL_PRICE_PLACES


class EmployeePosition(models.Model):
    Meta = named_meta('Должность', 'EmployeePosition')

    title = models.CharField(max_length=256, help_text='Наименование', null=False, unique=True)


class CustomUser(PermissionsMixin):
    Meta = named_meta('Пользователь', 'CustomUser')

    phone = models.CharField(max_length=64, help_text='Номер телефона', null=True)
    email = models.EmailField(max_length=254, help_text='Почтовый адрес', null=True)
    is_superuser = models.BooleanField(default=False, help_text='Наличие прав суперпользователя', null=False)

    password = models.CharField(help_text='password', max_length=128, null=True, blank=True)
    last_login = models.DateTimeField(help_text='last login', blank=True, null=True)

    employee_position = models.ForeignKey(
        EmployeePosition,
        help_text='Пользователи -> Должность',
        related_name='users',
        null=True,
        on_delete=models.SET_NULL,
    )
    name = models.CharField(max_length=128, help_text='Ф.И.О.', null=True)

    groups = models.ManyToManyField(
        Group, help_text='Пользователи <-> Группы', related_name='custom_user', related_query_name='user', blank=True
    )

    USERNAME_FIELD = 'username'
    EMAIL_FIELD = 'email'

    username_validator = UnicodeUsernameValidator()

    username = models.CharField(
        help_text='username, Required. 150 characters or fewer. Letters, digits and @/./+/-/_ only.',
        max_length=150,
        unique=True,
        validators=[username_validator],
    )
    first_name = models.CharField(help_text='first name', max_length=150, blank=True, null=True)
    last_name = models.CharField(help_text='last name', max_length=150, blank=True, null=True)
    is_staff = models.BooleanField(
        help_text='staff status. Designates whether the user can log into this admin site.',
        default=False,
    )
    is_active = models.BooleanField(
        help_text='active Designates whether this user should be treated as active. Unselect this instead of deleting accounts.',
        default=True,
    )
    date_joined = models.DateTimeField(help_text='date joined', default=timezone.now)

    sales_year_plan = models.DecimalField(
        help_text='План по продажам на год',
        max_digits=20,
        null=True,
        decimal_places=DECIMAL_PRICE_PLACES
    )

    objects = UserManager()

    REQUIRED_FIELDS = ['email']

    def clean(self):
        setattr(self, self.USERNAME_FIELD, self.normalize_username(self.get_username()))
        self.email = self.__class__.objects.normalize_email(self.email)

    def get_full_name(self):
        """
        Return the first_name plus the last_name, with a space in between.
        """
        full_name = '%s %s' % (self.first_name, self.last_name)
        return full_name.strip()

    def get_short_name(self):
        """Return the short name for the user."""
        return self.first_name

    def email_user(self, subject, message, from_email=None, **kwargs):
        """Send an email to this user."""
        send_mail(subject, message, from_email, [self.email], **kwargs)

    def __str__(self):
        return self.get_username()

    def save(self, *args, **kwargs):
        if getattr(self, '_password', None) is not None:
            password_validation.password_changed(self._password, self)
            self._password = None
        prev_state = CustomUser.objects.filter(id=self.id).first()
        if not prev_state or prev_state and self.password != prev_state.password:
            self.set_password(self.password)
        super().save(*args, **kwargs)
        if not prev_state or prev_state and self.password != prev_state.password:
            PreviousPassword.objects.create(user=self, password_hash=self.password)

    def get_username(self):
        """Return the username for this User."""
        return getattr(self, self.USERNAME_FIELD)

    def natural_key(self):
        return (self.get_username(),)

    @property
    def is_anonymous(self):
        """
        Always return False. This is a way of comparing User objects to
        anonymous users.
        """
        return False

    @property
    def is_authenticated(self):
        """
        Always return True. This is a way to tell if the user has been
        authenticated in templates.
        """
        return True

    def set_password(self, raw_password):
        self.password = make_password(raw_password)
        self._password = raw_password

    def check_password(self, raw_password):
        """
        Return a boolean of whether the raw_password was correct. Handles
        hashing formats behind the scenes.
        """

        def setter(raw_password):
            self.set_password(raw_password)
            # Password hash upgrades shouldn't be considered password changes.
            self._password = None
            self.save(update_fields=['password'])

        return check_password(raw_password, self.password, setter)

    def set_unusable_password(self):
        # Set a value that will never be a valid hash
        self.password = make_password(None)

    def has_usable_password(self):
        """
        Return False if set_unusable_password() has been called for this user.
        """
        return is_password_usable(self.password)

    def _legacy_get_session_auth_hash(self):
        # RemovedInDjango40Warning: pre-Django 3.1 hashes will be invalid.
        key_salt = 'django.contrib.auth.models.AbstractBaseUser.get_session_auth_hash'
        return salted_hmac(key_salt, self.password, algorithm='sha1').hexdigest()

    def get_session_auth_hash(self):
        """
        Return an HMAC of the password field.
        """
        key_salt = 'django.contrib.auth.models.AbstractBaseUser.get_session_auth_hash'
        return salted_hmac(
            key_salt,
            self.password,
            # RemovedInDjango40Warning: when the deprecation ends, replace
            # with:
            # algorithm='sha256',
            algorithm=settings.DEFAULT_HASHING_ALGORITHM,
        ).hexdigest()

    @classmethod
    def get_email_field_name(cls):
        try:
            return cls.EMAIL_FIELD
        except AttributeError:
            return 'email'

    @classmethod
    def normalize_username(cls, username):
        return unicodedata.normalize('NFKC', username) if isinstance(username, str) else username


class PreviousPassword(models.Model):
    user = models.ForeignKey(
        CustomUser,
        on_delete=models.CASCADE,
        help_text='Пользователь',
        related_name='previous_passwords',
    )
    password_hash = models.CharField(
        help_text='Хеш',
        max_length=128,
        null=True,
        blank=True,
    )


def user_password_is_used(user, raw_password):
    is_used = False

    lim = None if settings.PREVENT_PASSWORD_REUSE == 'ALL' else settings.PREVENT_PASSWORD_REUSE

    old_passwords = PreviousPassword.objects.filter(user=user) \
        .order_by('-id')[slice(None, lim, None)] \
        .values_list('password_hash', flat=True)
    for password in old_passwords:
        # raw_pass_hash = make_password(raw_password)
        # на данный момент check_password всегда возвращает False
        # нужно будет дополнить, когда будут решены вопросы с аутентификацией
        if check_password(raw_password, password):
            is_used = True
            break
    return is_used, lim
