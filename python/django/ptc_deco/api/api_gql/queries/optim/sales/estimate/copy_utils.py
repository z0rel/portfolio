def _copy_model_attributes(dst, src, mask_fields):
    if src is None or dst is None:
        return dst

    for f in src._meta.get_fields():
        if f.name not in mask_fields:
            setattr(dst, f.name, getattr(src, f.name))
    dst.pk = src.id

    return dst


def _copy_from_slots(dst, src, mask_fields):
    if src is None or dst is None:
        return dst

    for f in src.__slots__:
        if f not in mask_fields:
            setattr(dst, f, getattr(src, f))
    dst.pk = src.id

    return dst


def _copy_model_attributes_mapped(dst, src, mapped_fields):
    if src is None or dst is None:
        return dst

    for f_dst, f_src in mapped_fields.items():
        attr = getattr(src, f_src)
        setattr(dst, f_dst, attr)

    dst.pk = src.id

    return dst


def _copy_model_attributes_from_dict(dst, src, mapped_fields):
    if src is None or dst is None:
        return dst

    for f_dst, f_src in mapped_fields.items():
        setattr(dst, f_dst, src[f_src])
    return dst
