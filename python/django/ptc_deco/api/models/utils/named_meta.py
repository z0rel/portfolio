from ...morpher import to_plural_form


def named_meta(verbose_name, future_class_name, **kwargs):
    # Django переписывает метагенератор. Текущий метагенератор никогда не вызывается
    def meta_type_generator(future_class_name):
        if 'verbose_name_plural' in kwargs:
            verbose_name_plural = kwargs.pop('verbose_name_plural')
        else:
            verbose_name_plural = to_plural_form(verbose_name)
        meta_attrs = {
            'verbose_name': verbose_name,
            'verbose_name_plural': verbose_name_plural,
            **kwargs
        }
        return type(f'{future_class_name}.Meta', (), meta_attrs)

    return meta_type_generator(future_class_name)



