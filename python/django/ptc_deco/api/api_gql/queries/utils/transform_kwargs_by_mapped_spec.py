from .ids import convert_ID_to_id


def default_transforormer_fn(kwargs, k, v):
    kwargs[k] = v


def transform_kwargs_by_mapped_spec(mapped_spec, kwargs, dst_kwargs, transformer_fn=default_transforormer_fn):
    for (src_key, (dst_key, is_id)) in mapped_spec.items():
        item = kwargs.pop(src_key, None)
        if item is not None:
            if is_id:
                item = convert_ID_to_id(item)
            transformer_fn(dst_kwargs, dst_key, item)
