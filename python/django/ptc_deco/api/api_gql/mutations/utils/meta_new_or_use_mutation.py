from enum import Enum

from django.conf import settings as app_settings
from ....morpher import to_accs_first, to_gent, to_accs_all
from graphene.utils.str_converters import to_camel_case


def _many_to_many_extras_ops(operation, name_of_base_type):
    return {
        'add': {'type': 'ID', 'operation': 'add'},
        'create': {'type': f'{operation}{name_of_base_type}Input', 'operation': 'add'},
        'remove': {'type': 'ID'},
    }


def get_first_file(value, info):
    for (k, v) in info.context.FILES.items():
        # base, ext = os.path.splitext(v.name)
        # v.name = base64.b64encode(encoding.smart_bytes(v.name)).decode('ascii')
        return v
    return value


class LeStr:
    name = '__doc__'

    def __str__(self):
        return ''

    def __lt__(self, other):
        return False

    def __gt__(self, other):
        return False


class ClassMethod(object):
    'Emulate PyClassMethod_Type() in Objects/funcobject.c'

    def __init__(self, f):
        self.f = f

    def __get__(self, obj, klass=None):
        if klass is None:
            klass = type(obj)

        def newfunc(*args):
            return self.f(klass, *args)

        return newfunc


def _handle_first_file(cls, value, name, info):
    return get_first_file(value, info)


class CudListNode:
    def __init__(
        self,
        query_name,
        model_class,
        file_fields=None,
        fk_fields=None,
        m2m_fields=None,
        m2o_fields=None,
        o2o_fields=None,
        options=None,
        type_name=None,
        batch=False,
        to_accs_all=False,
        nobatch=False,
        nopatch=False,
        custom_fields=None,
        before_create=None,
        before_update=None,
        before_mutate=None,
        validate=None,
        login_required=False,
        permissions=None,
        create_permissions=None,
        update_permissions=None,
        delete_permissions=None,
    ):
        self.query_name = query_name
        self.model_class = model_class
        self.file_fields = file_fields
        self.fk_fields = fk_fields
        self.m2m_fields = m2m_fields
        self.m2one_fields = m2o_fields
        self.o2o_fields = o2o_fields
        self.options = options
        self.type_name = type_name
        self.batch = batch
        self.to_accs_all = to_accs_all
        self.nobatch = nobatch
        self.nopatch = nopatch
        self.custom_fields = custom_fields
        self.before_create = before_create
        self.before_update = before_update
        self.before_mutate = before_mutate
        self.validate = validate
        self.login_required = login_required
        self.permissions = permissions
        self.create_permissions = create_permissions
        self.update_permissions = update_permissions
        self.delete_permissions = delete_permissions


class NameOfMutationFlags(Enum):
    CREATE = 'create'
    CREEATE_BATCH = 'create_batch'
    UPDATE = 'update'
    UPDATE_BATCH = 'update_batch'
    UPDATE_PATCH = 'update_patch'
    UPDATE_BATCH_PATCH = 'update_batch_patch'
    DELETE = 'delete'
    DELETE_BATCH = 'delete_batch'


FLAGS_ACCEPT_M2M = {
    NameOfMutationFlags.CREATE,
    NameOfMutationFlags.UPDATE,
}

FLAGS_IS_DELETE = {
    NameOfMutationFlags.DELETE,
    NameOfMutationFlags.DELETE_BATCH,
}


def _meta_create_mutation(
    crud_prefix,
    future_class_name,
    future_class_parents,
    settings: CudListNode,
):
    is_delete = False
    future_class_attr = {}
    meta_attrs = {'model': settings.model_class}
    _meta = settings.model_class._meta

    login_required = not app_settings.DISABLE_GQL_AUTH_CONTROL
    meta_attrs['login_required'] = settings.login_required and login_required

    if crud_prefix in [
        NameOfMutationFlags.CREATE,
        NameOfMutationFlags.CREEATE_BATCH,
    ]:
        meta_attrs['permissions'] = settings.create_permissions if login_required else None
    elif crud_prefix in [
        NameOfMutationFlags.UPDATE,
        NameOfMutationFlags.UPDATE_BATCH,
        NameOfMutationFlags.UPDATE_PATCH,
        NameOfMutationFlags.UPDATE_BATCH_PATCH,
    ]:
        meta_attrs['permissions'] = settings.update_permissions if login_required else None
    elif crud_prefix in [
        NameOfMutationFlags.DELETE,
        NameOfMutationFlags.DELETE_BATCH,
    ]:
        meta_attrs['permissions'] = settings.delete_permissions if login_required else None
    else:
        meta_attrs['permissions'] = (settings.permissions if settings.permissions else ()) if login_required else ()

    if settings.type_name:
        meta_attrs['type_name'] = settings.type_name.get(crud_prefix.value, None)

    if settings.file_fields and crud_prefix not in FLAGS_IS_DELETE:
        handler = ClassMethod(_handle_first_file)
        for field_name in settings.file_fields:
            future_class_attr[f'handle_{field_name}'] = handler

    cap_crud_prefix = to_camel_case(crud_prefix.value).capitalize()

    if settings.m2m_fields and crud_prefix in FLAGS_ACCEPT_M2M:
        meta_attrs['many_to_many_extras'] = {
            field_name: _many_to_many_extras_ops(cap_crud_prefix, class_name)
            for (field_name, class_name) in settings.m2m_fields
        }

    if settings.o2o_fields and crud_prefix in FLAGS_ACCEPT_M2M:
        meta_attrs['one_to_one_extras'] = {
            field_name: {'type': f'{cap_crud_prefix}{class_name}Input'}
            for (field_name, class_name) in settings.o2o_fields
        }
    if settings.fk_fields and crud_prefix in FLAGS_ACCEPT_M2M:
        meta_attrs['foreign_key_extras'] = {
            field_name: {'type': f'{cap_crud_prefix}{class_name}Input'}
            for (field_name, class_name) in settings.fk_fields
        }
    if settings.m2one_fields and crud_prefix in FLAGS_ACCEPT_M2M:
        meta_attrs['many_to_one_extras'] = {
            field_name: {
                'add': {'type': 'ID' if settings.query_name != 'reservation_nonrts' else 'auto'},
                'remove': {'type': 'ID'},
            }
            for (field_name, class_name) in settings.m2one_fields
        }
    if settings.custom_fields:
        meta_attrs['custom_fields'] = settings.custom_fields

    if settings.before_mutate:
        @classmethod
        def before_mutate(cls, root, info, input, *args):
            settings.before_mutate(cls, root, info, input, *args)

        future_class_attr['before_mutate'] = before_mutate

    if settings.validate:
        @classmethod
        def validate(cls, root, info, input, *args):
            settings.validate(cls, root, info, input, *args)

        future_class_attr['validate'] = validate
    node_prefix = ''
    to_accs = to_accs_first if not settings.to_accs_all else to_accs_all

    if crud_prefix == NameOfMutationFlags.CREATE:
        node_prefix = 'Данные для создания '
        meta_attrs['description'] = f'Создать {to_accs(_meta.verbose_name)}'
        future_class_attr['description'] = f'Создать {to_accs(_meta.verbose_name)}'
    elif crud_prefix == NameOfMutationFlags.CREEATE_BATCH:
        node_prefix = 'Данные для пакетного создания '
        meta_attrs['description'] = f'Создать пакетно {to_accs(_meta.verbose_name)}'
        future_class_attr['description'] = f'Создать пакетно {to_accs(_meta.verbose_name)}'
    elif crud_prefix == NameOfMutationFlags.UPDATE:
        node_prefix = 'Данные для обновления (все NOT NULL поля - обязательные)'
        meta_attrs['description'] = f'Обновить {to_accs(_meta.verbose_name)}. Все NOT NULL поля - обязательные'
        future_class_attr['description'] = f'Обновить {to_accs(_meta.verbose_name)}. Все NOT NULL поля - обязательные'
    elif crud_prefix == NameOfMutationFlags.UPDATE_BATCH:
        node_prefix = 'Данные для пакетного обновления (все NOT NULL поля - обязательные)'
        meta_attrs['description'] = f'Обновить пакетно {to_accs(_meta.verbose_name)}. Все NOT NULL поля - обязательные'
        future_class_attr[
            'description'
        ] = f'Обновить пакетно {to_accs(_meta.verbose_name)}. Все NOT NULL поля - обязательные'
    elif crud_prefix == NameOfMutationFlags.UPDATE_PATCH:
        node_prefix = 'Данные для обновления (все поля - необязательные)'
        meta_attrs['description'] = f'Обновить {to_accs(_meta.verbose_name)}. Все поля - необязательные'
        future_class_attr['description'] = f'Обновить {to_accs(_meta.verbose_name)}. Все поля - необязательные'
    elif crud_prefix == NameOfMutationFlags.UPDATE_BATCH_PATCH:
        node_prefix = 'Данные для пакетного обновления (все поля - необязательные)'
        meta_attrs['description'] = f'Обновить пакетно {to_accs(_meta.verbose_name)}. Все поля - необязательные'
        future_class_attr['description'] = f'Обновить пакетно {to_accs(_meta.verbose_name)}. Все поля - необязательные'
    elif crud_prefix == NameOfMutationFlags.DELETE:
        node_prefix = 'Данные для удаления '
        meta_attrs['description'] = f'Удалить {to_accs(_meta.verbose_name)}'
        future_class_attr['description'] = f'Удалить {to_accs(_meta.verbose_name)}'
    elif crud_prefix == NameOfMutationFlags.DELETE_BATCH:
        node_prefix = 'Данные для пакетного удаления '
        meta_attrs['description'] = f'Удалить пакетно {to_accs(_meta.verbose_name)}'
        future_class_attr['description'] = f'Удалить пакетно {to_accs(_meta.verbose_name)}'

    future_class_attr['Meta'] = type(f'{future_class_name}.Meta', (), meta_attrs)

    class ParentType(*future_class_parents):
        class Meta:
            abstract = True

        @classmethod
        def __init_subclass_with_meta__(cls, **kwargs):
            # Документирование узла параметров мутаций создания и удаления
            field_types = {'__doc__': node_prefix + to_gent(settings.model_class._meta.verbose_name)}
            old_meta = kwargs['model']._meta
            old_fields = old_meta.fields
            old_meta.fields = old_meta.fields + (LeStr(),)
            super().__init_subclass_with_meta__(field_types=field_types, **kwargs)
            old_meta.fields = old_fields

    return type(future_class_name, (ParentType,), future_class_attr)


def get_mutation_class_name(crud_prefix, query_name):
    return f'{to_camel_case(crud_prefix.value).capitalize()}{to_camel_case(query_name).capitalize()}Mutation'


def meta_new_or_use_mutation(attrs, crud_prefix, django_mutation_class, settings: CudListNode):
    attrs[f'{crud_prefix.value}_{settings.query_name}'] = _meta_create_mutation(
        crud_prefix=crud_prefix,
        future_class_name=get_mutation_class_name(crud_prefix, settings.query_name),
        future_class_parents=(django_mutation_class,),
        settings=settings,
    ).Field()
