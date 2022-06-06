import dateFormat from 'dateformat';

export const getProjectAppendices = (dataReservations, {pageInfo, orderBy}) => {
  let { data, loading, error, refetch } = dataReservations;
  return {
    loading,
    error,
    refetch,
    totalCount: data?.searchAppendix?.pageInfo?.totalCount,
    pageInfo: pageInfo[0],
    setPageInfo: pageInfo[1],
    orderBy: orderBy[0],
    setOrderBy: orderBy[1],
    mappedData:
      (data?.searchAppendix?.edges &&
        data.searchAppendix.edges.map((item) => ({
          id: item?.node?.id,
          key: item?.node?.id,
          appendix_code: '#' + item?.node?.code,
          appendix_summa: '', // item.node.additionalCosts.edges[0].node.summa, TODO:
          appendix_createDate: dateFormat(item?.node?.createdDate, 'dd.mm.yyyy'),
          appendix_reservDates: `${dateFormat(item?.node?.periodStartDate, 'dd.mm.yyyy')} – ${dateFormat(
            item?.node?.periodEndDate,
            'dd.mm.yyyy',
          )}`,
          dateForRouter: [item?.node?.dateFrom, item?.node?.dateTo],
          creator: item?.node?.creator,
          paymentDate: item?.node?.paymentDate,
          salesManager: item?.node?.salesManager,
          signatoryOne: item?.node?.signatoryOne,
          signatoryPosition: item?.node?.signatoryPosition,
          signatoryTwo: item?.node?.signatoryTwo,
        }))) ||
      [],
  };
};
