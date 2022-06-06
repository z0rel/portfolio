from decimal import Decimal


def _discount_to_calc_val(discount):
    if discount is None:
        return Decimal(0)
    else:
        return (Decimal(100) - Decimal(discount)) / Decimal(100)


def calc_discount_val(discount_price_percent, default_discount_price_percent, default_discount_price_percent_val):
    if discount_price_percent is not None:
        return discount_price_percent, _discount_to_calc_val(discount_price_percent)
    elif default_discount_price_percent is not None:
        return default_discount_price_percent, default_discount_price_percent_val
    else:
        return Decimal(0), Decimal(1)


def calc_default_discount_percent(percent, percent_val, add_discount, add_discount_val):
    default_discount_val = percent_val
    default_discount = percent

    # Если default_discount_price_percent_val is None - попытаться взять значение скидки по прайсу из
    #  конфигурационных расходов для данного формата
    if default_discount is None:
        default_discount = add_discount
        default_discount_val = add_discount_val
    return default_discount, default_discount_val


class Discount:
    __slots__ = ('_default_discount_percent', '_default_discount_percent_val', 'discount_percent_calculated',
                 '_percent_calculated', 'value_after_discount_calculated', 'discount_percent_selected_val',
                 'value_after_discount_selected', 'discount_percent_selected')

    def __init__(self, percent, percent_val, add_discount, add_discount_val,
                 discount_price_percent,  # r.discount_price_percent_setted
                 value_before_discount,  # r.rent_by_price_calculated
                 value_after_discount_setted,  # r.rent_by_price_after_discount_setted
                 ):
        # Скидка по умолчанию
        self._default_discount_percent = percent
        self._default_discount_percent_val = percent_val

        # Если default_discount_price_percent_val is None - попытаться взять значение скидки по прайсу из
        #  конфигурационных расходов для данного формата
        if self._default_discount_percent is None:
            self._default_discount_percent = add_discount
            self._default_discount_percent_val = add_discount_val

        # Расчетный и обратный % скидки
        if discount_price_percent is not None:
            self.discount_percent_calculated = discount_price_percent
            self._percent_calculated = _discount_to_calc_val(discount_price_percent)
        elif self._default_discount_percent is not None:
            self.discount_percent_calculated = self._default_discount_percent
            self._percent_calculated = self._default_discount_percent_val
        else:
            self.discount_percent_calculated = Decimal(0)
            self._percent_calculated = Decimal(1)

        if self._percent_calculated is not None and value_before_discount is not None:
            self.value_after_discount_calculated = self._percent_calculated * value_before_discount
        else:
            self.value_after_discount_calculated = value_before_discount

        # Выбранный обратный % скидки, выбранное значение после скидки и выбранный прямой % скидки
        if value_after_discount_setted is None:
            self.discount_percent_selected_val = self._percent_calculated
            self.value_after_discount_selected = self.value_after_discount_calculated
            self.discount_percent_selected = self.discount_percent_calculated
        else:
            if value_before_discount is not None:
                self.discount_percent_selected_val = value_after_discount_setted / value_before_discount
                self.discount_percent_selected = (Decimal(1) - self.discount_percent_selected_val) * Decimal(100)
            else:
                self.discount_percent_selected_val = None
                self.discount_percent_selected = None

            self.value_after_discount_selected = value_after_discount_setted

