import React, { useState } from 'react';
import { useQuery } from '@apollo/client';
import styled from 'styled-components';
import dateFormat from 'dateformat';

import Tablea from '../../../components/Tablea/Tablea_estimate';
import { column } from '../../../components/Table/utils';
import { QUERY_INVOICE } from './query_invoice';
import { ErrorComponent } from '../../../components/Logic/ErrorComponent';
import { fmtPrice } from '../Estimate/utils/utils';


const columns = [
  column('Код проекта', 'code', 60, true, true),
  column('Код приложения', 'appendixCode', 60, true, true),
  column('Наименование проекта', 'title', 90, true, true),
  column('Бренд', 'brand', 80, true, true),
  column('Дата начала', 'startDate', 90, true, true),
  column('Рекламодатель', 'advertiser', 90, true, true),

  column('Рекламное агенство', 'advertAgency', 90, true, true),
  column('Город', 'city', 80, true, true),
  column('Сумма без НДС', 'sumWithoutNds', 80, true, true),
  column('Общая сумма', 'wholeSum', 80, true, true),
  column('Способ оплаты', 'customerPaymentMethod', 80, true, true),
  column('Оплатить не позднее', 'paymentLastDate', 80, true, true),
  column('АВР', 'avr', 80, true, true),


];


const mapQueryData = (data, loading) => (loading ? [] : data?.searchSalesInvoice?.edges.map((item) => ({
  code: item?.node?.appendix?.project?.code,
  appendixCode: item?.node?.appendix?.code,
  title: item?.node?.appendix?.project?.title,
  brand: item?.node?.appendix?.project?.brand?.title,
  startDate: item?.node?.appendix?.project?.startDate ? dateFormat(item?.node?.project?.startDate, 'dd.mm.yyyy') : null,
  advertiser: item?.node?.appendix?.project?.client?.title,
  advertAgency: item.node?.appendix?.project?.agency?.title,
  sumWithoutNds: fmtPrice(item.node?.sumWithoutNds),
  wholeSum: fmtPrice(item.node?.wholeSum),
  customerPaymentMethod: item?.node?.customerPaymentMethod,
  paymentLastDate: item?.node?.paymentLastDate ? dateFormat(item?.node?.paymentLastDate, 'dd.mm.yyyy') : null,
  avr: item.node?.avr ? 'Да' : 'Нет',
})) || []);


const PanelDesign = () => {
  const filter = {}

  const {loading, error, data, refetch} = useQuery(QUERY_INVOICE, { variables: filter });
  const [columnsForPopup, setColumnsForPopup] = useState(columns);

  if (error)
    return <ErrorComponent error={error}/>;

  return (
    <StyledTableBar>
      <Tablea
        key="invoices"
        columns={columnsForPopup}
        setColumns={setColumnsForPopup}
        data={mapQueryData(data, loading)}
        select={true}
        edit={true}
        del={false}
        loading={loading}
        enableChoosePeriod={false}
        notheader={false}
        refetch={refetch}
        // setDeleted={setDeleted}
      />
    </StyledTableBar>
  );
};

const StyledTableBar = styled.div`
  width: 100%;
  .design-info {
    border-radius: 8px;
    border: 1px solid #d3dff0;
  }
`

export default PanelDesign;
