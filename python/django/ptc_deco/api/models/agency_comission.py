from django.db import models
from .utils import DECIMAL_PRICE_PLACES, named_meta


# Агентская комиссия присваивается следующим образом:
# 1. Пользователь выбирается распространяется ли АК на сумму с НДС или на сумму без НДС.
# 2. Затем пользователь выбирает тип АК – в процентах или конкретная сумма.
# 3. Пользователь выбирает на какие услуги распространяется
# АК: все аренда налог печать монтаж доп.расходы  НОН РТС(распространяется только на маржу)
# 4. АК может быть рассчитана либо от факта выбранных услуг, либо пересчетом от прайсовой стоимости.
# АК распространяется не на все проекты, к которым присвоен контрагент с АК.
# Для того, чтобы АК была присвоена к проекту, необходимо найти контрагента, раскрыть список проектов, связанных
# с контрагентом, и присвоить сумму / скидку АК к данному проекту.Можно присвоить данные как к проекту
# в целом, так и к конкретному периоду внутри проекта.
# Сумма и процент агентской комиссии в проектах по умолчанию скрыт, при необходимости данная информация может быть
# отображена.
# При выставлении счёта, АК не учитывается.
# Можно присвоить агентскую комиссию как к проекту в целом, так и к конкретному периоду внутри проекта.

class AgencyCommission(models.Model):
    Meta = named_meta('Агентская комиссия', 'AgencyCommission')
    
    to_rent = models.BooleanField(help_text='Агентская комиссия распространяется на аренду', default=False)
    to_nalog = models.BooleanField(help_text='Агентская комиссия распространяется на налог', default=False)
    to_print = models.BooleanField(help_text='Агентская комиссия распространяется на печать', default=False)
    to_mount = models.BooleanField(help_text='Агентская комиссия распространяется на монтаж', default=False)
    to_additional = models.BooleanField(help_text='Агентская комиссия распространяется на доп. расходы', default=False)
    to_nonrts = models.BooleanField(help_text='Агентская комиссия распространяется на маржу НОН РТС', default=False)

    percent = models.DecimalField(help_text='Процент агентской комисии', max_digits=20, null=True, 
                                  decimal_places=DECIMAL_PRICE_PLACES)

    value = models.DecimalField(help_text='Сумма агентской комиссии', max_digits=20, null=True, 
                                decimal_places=DECIMAL_PRICE_PLACES)

    agent = models.ForeignKey(
        'Partner',
        help_text='Агент для перечисления агентской коммиссии',
        null=True,
        on_delete=models.deletion.SET_NULL
    )

    def __str__(self):
        return (f'id: {self.id}, '
                f'rent: {self.to_rent}, nalog: {self.to_nalog}, print: {self.to_print} ' +
                f'mount: {self.to_mount}, additional: {self.to_additional}, nonrts: {self.to_nonrts} '
                f'percent: {self.percent}, value: {self.value}')

    def save(self, *args, **kwargs):
        if self.percent is not None and self.value is not None:
            raise Exception('Значение процента и суммы агентской комиссии не могут быть заданы одновременно')
        super().save(*args, **kwargs)


def discount_mixin_factory(help_text_field, superclass=models.Model):
    class DiscountMixin(superclass):
    
        discount_percent = models.DecimalField(help_text=f'Процент скидки {help_text_field}', max_digits=20,
                                               null=True, decimal_places=DECIMAL_PRICE_PLACES)
    
        cost_after_discount = models.DecimalField(help_text=f'Стоимость после скидки {help_text_field}', max_digits=20,
                                                  null=True, decimal_places=DECIMAL_PRICE_PLACES)
        
        class Meta:
            abstract = True
        
        def save(self, *args, **kwargs):
            if self.discount_percent is not None and  self.cost_after_discount is not None:
                raise Exception("Скидка не может быть задана как % и значением после скидки одновременно")
            
            super().save(*args, **kwargs)
            
    return DiscountMixin


class DiscountMixinPartnerAndProject(models.Model):
    discount_price_percent = models.DecimalField(help_text=f'Процент скидки по прайсу', max_digits=20,
                                                 null=True, decimal_places=DECIMAL_PRICE_PLACES)

    discount_client_percent = models.DecimalField(help_text=f'Процент скидки на клиента', max_digits=20,
                                                 null=True, decimal_places=DECIMAL_PRICE_PLACES)

    discount_nalog_percent = models.DecimalField(help_text=f'Процент скидки на налог', max_digits=20,
                                                 null=True, decimal_places=DECIMAL_PRICE_PLACES)

    class Meta:
        abstract = True


