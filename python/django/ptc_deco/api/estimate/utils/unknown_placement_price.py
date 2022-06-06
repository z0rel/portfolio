from ptc_deco.api.models import Format, City


class UnknownPlacementPrice(Exception):
    def __init__(self, city_id, format_id, period_id: int=None, message='Не задана прайсовая стоимость (PlacementPrice)'):
        format_value: Format
        format_value = Format.objects.filter(id=format_id)[0]
        city = City.objects.filter(id=city_id)[0]
        self.format_value = format_value
        self.period_id = period_id
        self.city = city
        self.message = message

    def __str__(self):
        city = self.city
        format_value = self.format_value
        result = (
            f'{self.message} для города "{city.title}" id={city.id}, '
            f'формата "{format_value.title}" id={format_value.id}, '
            f'модели "{format_value.model.title}" id={format_value.model.title}, '
            f'подсемейства "{format_value.model.underfamily.title} id={format_value.model.underfamily.id}", '
            f'семейства "{format_value.model.underfamily.family.title}" id={format_value.model.underfamily.family.id}'
        )
        if self.period_id is not None:
            result += f' периода={self.period_id}'

        return result
