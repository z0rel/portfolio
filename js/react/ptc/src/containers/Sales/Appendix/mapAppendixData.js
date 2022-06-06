import { fmtPeriod, fmtPrice0, fmtPriceNum0, fmtPercent } from '../Estimate/utils/utils';
import { estimate } from '../../../assets/proto_compiled/proto';
import { base64ToArrayBuffer } from '../../../components/Logic/base64ToArrayBuffer';

let trDate = (v) => (v ? new Date(v).toLocaleDateString() : '');

export const mapAppendixData = (data) => {
  let appendixData = estimate.AddressProgramm.decode(
    base64ToArrayBuffer(data),
  );
  const mappedData = {
    code: appendixData?.appendix?.code || '',
    createdDate: trDate(appendixData.appendix.createdDate),
    startedDate: trDate(appendixData.appendix.periodStartDate),
    endDate: trDate(appendixData.appendix.periodEndDate),
    contractCode: appendixData.appendix.contract_Code || '',
    contractDate: trDate(appendixData.appendix.contract_RegistrationDate),
    paymentDate: trDate(appendixData.appendix.paymentDate),
    signatoryOne: appendixData.appendix.signatoryOne || '',
    signatoryPosition: appendixData.appendix.signatoryPosition || '',
    smetaSummary: fmtPrice0(appendixData.itogs.summaryEstimateValue),
    addressProgramm:
      appendixData.items.map((item) => {
        return {
          key: item.id,
          format: item.formatTitle,
          city: item.cityTitle,
          period: fmtPeriod(item.dateFrom, item.dateTo),
          renta: fmtPrice0(item.rent),
          rentaWithDiscount: fmtPrice0((item.rent || 0) - (item.discountClientValue || 0)),
          rentaDiscount: fmtPercent(item.discountClientPercent || 0),
          print: fmtPrice0(item.printing),
          install: fmtPrice0(item.mounting),
          addexpense: fmtPrice0(item.additional),
          nalog: fmtPrice0(item.nalog),
          nalogDiscount: fmtPercent(item.nalogDiscountPercent),
          nalogWithDiscount: fmtPrice0((item.nalog || 0) - (item.nalogDiscountValue || 0)),
          amount: fmtPrice0(item.itogSummary),
        };
      }) || [],
  };
  const sliderCountData = getSliderCountData(appendixData);
  return {appendixData, mappedData, sliderCountData};
}


export const getSliderCountData = (estimateItogs) => {
  return {
    info: [
      { title: 'Номер договора', data: estimateItogs?.appendix.contract_SerialNumber || '' },
      { title: 'Номер приложения', data: estimateItogs?.appendix.code || '' },
      {
        title: 'Реквизиты',
        data: `БИН: ${estimateItogs?.appendix.project_Client_BinNumber || ''}; БИК: ${
          estimateItogs?.appendix.project_Client_Bik || ''
        }`,
      },
      { title: 'Дата договора', data: trDate(estimateItogs?.appendix.contract_RegistrationDate) },
    ],
    count: [
      { title: 'Налог', data: fmtPriceNum0(estimateItogs?.itogs.nalogAfterDiscount) },
      { title: 'Аренда со скидкой', data: fmtPriceNum0(estimateItogs?.itogs.rentToClientDiscounted) },
      { title: 'Доп. расходы', data: fmtPriceNum0(estimateItogs?.itogs.staticAdditional) },
      { title: 'Монтаж', data: fmtPriceNum0(estimateItogs?.itogs.staticMounting) },
      { title: 'Печать', data: fmtPriceNum0(estimateItogs?.itogs.staticPrinting) },
    ],
  };
};
