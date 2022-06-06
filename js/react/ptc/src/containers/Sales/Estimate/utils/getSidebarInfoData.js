import collapseIcon from '../../../../img/collapse-icon.svg';
import { fmtPercent, fmtPrice, fmtPrice0, capitalize } from './utils';

export const getSidebarInfoData = (data) => {
  return [
    {
      id: 1, title: 'Аренда', icon: collapseIcon, isShowed: true, sumBlock: false,
      content: [
        { title: 'Аренда по прайсу:',            value: fmtPrice0(data?.itogs.rentByPrice) },
        { title: 'Скидка на аренду по прайсу:',  value: fmtPercent(data?.itogs.rentByPriceDiscountPercent) },
        { title: 'Аренда на клиента:',           value: fmtPrice0(data?.itogs.rentToClient) },
        { title: 'Скидка на аренду на клиента:', value: fmtPercent(data?.itogs.rentToClientDiscountPercent) }
      ]
    },
    {
      id: 2, title: 'Доп. работы', icon: collapseIcon, isShowed: true, sumBlock: false,
      content: [
        { title: 'Монтаж:',      value: fmtPrice0(data?.itogs.staticMounting) },
        { title: 'Печать:',      value: fmtPrice0(data?.itogs.staticPrinting) },
        { title: 'Доп. работы:', value: fmtPrice0(data?.itogs.staticAdditional) },
      ]
    },
    {
      id: 3, title: 'Доп. расходы', icon: collapseIcon, isShowed: true, sumBlock: false,
      content: (
        data?.additionalRtsByTitle.map((item, index) => ({
            title: capitalize(item.name),
            value: fmtPrice(item.summaAfterDiscount),
          })
        ) || [])
    },
    {
      id: 4, title: 'Агентская комиссия', icon: collapseIcon, isShowed: true, sumBlock: false,
      content: [
        { title: 'Процент АК:',          value: fmtPercent(data?.itogs.agencyCommissionPercent) },
        { title: 'Сумма АК:',            value: fmtPrice0(data?.itogs.agencyCommissionValue) },
        { title: 'Сумма за вычетом АК:', value: fmtPrice0(data?.itogs.summaryEstimateValueWithoutAgencyCommission) }
      ]
    },
    {
      id: 5, title: 'Налоги', icon: collapseIcon, isShowed: true, sumBlock: false,
      content: [
        { title: 'Налог:',              value: fmtPrice0(data?.itogs.nalogBeforeDiscount) },
        { title: 'Скидка на налог:',    value: fmtPercent(data?.itogs.nalogDiscountPercent) },
        { title: 'Налог после скидки:', value: fmtPrice0(data?.itogs.nalogAfterDiscount) }
      ]
    },
    {
      id: 6,
      title: 'НОН РТС',
      icon: collapseIcon,
      isShowed: true,
      sumBlock: false,
      content: (
        data?.additionalNonrtsByTitle.map((item, index) => ({
            title: capitalize(item.name),
            value: fmtPrice0(item.sale),
          })
        ) || [])
    },
    {
      id: 7,
      title: 'ИТОГО',
      icon: collapseIcon,
      isShowed: false,
      sumBlock: true,
      value: fmtPrice0(data?.itogs.summaryEstimateValue)
    }
  ];
};
