from .GetOrCreate import GetOrCreate
from ptc_deco.api import models as m


def get_default_country(get_or_create: GetOrCreate) -> m.Country:
    return get_or_create(m.Country, title='Республика Казахстан')
