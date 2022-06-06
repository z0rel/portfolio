class GetOrCreate:
    def __init__(self):
        self.cache = {}
        self.update_cache = {}

    def __call__(self, _model, **kwargs):
        raise_exc = 'raiseExc' in kwargs
        if raise_exc:
            kwargs.pop('raiseExc')

        key = (_model._meta.model_name, *sorted(kwargs.items()))
        try:
            result = self.cache[key]
            return result
        except KeyError:
            pass

        obj = _model.objects.filter(**kwargs)
        if obj:
            result = obj[0]
        else:
            if raise_exc:
                raise Exception('Object not exists in base ' + str(kwargs))
            result = _model.objects.create(**kwargs)

        self.cache[key] = result
        return result

    def update(self, modelname, obj, **kwargs):
        key = tuple(sorted(kwargs.items()))
        try:
            old_key = self.cache[modelname]
            if old_key == key:
                return
        except KeyError:
            pass

        obj.save(update_fields=kwargs.keys())
        self.cache[modelname] = key
