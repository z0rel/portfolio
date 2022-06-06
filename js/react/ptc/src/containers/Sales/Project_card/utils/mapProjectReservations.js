import { getConstructionSideCode } from '../../../../components/Logic/constructionSideCode';
import dateFormat from 'dateformat';

export const mapProjectReservations = (dataReservations, {pageInfo, orderBy}) => {
  let { data, loading, error, refetch } = dataReservations;
  return {
    loading,
    error,
    refetch,
    totalCount: data?.searchReservation?.pageInfo?.totalCount,
    pageInfo: pageInfo[0],
    setPageInfo: pageInfo[1],
    orderBy: orderBy[0],
    setOrderBy: orderBy[1],
    mappedData:
      (data?.searchReservation?.edges &&
        data?.searchReservation?.edges.map((item) => ({
          id: item?.node?.id,
          id_mapped: item?.node?.id ? atob(item?.node?.id).split(':')[1] : null,
          key: item?.node?.id,
          reservation_code: getConstructionSideCode(item?.node?.constructionSide),
          reservation_city: item.node?.constructionSide?.construction?.location?.postcode?.district?.city?.title || '',
          reservation_address: item.node?.constructionSide?.construction?.location?.marketingAddress?.address || '',
          reservation_format: item.node?.constructionSide?.advertisingSide?.side?.format?.title || '',
          reservation_side: item.node?.constructionSide?.advertisingSide?.title || '',
          reservation_creationDate: dateFormat(item?.node?.creationDate, 'dd.mm.yyyy'),
          reservation_startDate: dateFormat(item?.node?.dateFrom, 'dd.mm.yyyy'),
          reservation_expirationDate: dateFormat(item?.node?.dateTo, 'dd.mm.yyyy'),
          reservation_status: item?.node?.reservationType?.title,
          reservation_lighting: item.node?.constructionSide?.statusConnection ? 'Да' : 'Нет',
          reservation_branding: item.node?.branding ? 'Да' : 'Нет',
          reservation_package: item.node?.constructionSide?.package?.title || '',
          reservationTypeId: item?.node?.reservationType?.id,
          dateFrom: item?.node?.dateFrom,
          dateTo: item?.node?.dateTo,
          branding: item.node?.branding,
        }))) ||
      [],
  };
};
