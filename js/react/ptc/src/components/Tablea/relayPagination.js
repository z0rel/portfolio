import { arrayConnectionString } from '../Logic/arrayConnectionString';

export const onChangePage = (setPageInfo) => (page, pageSize) => {
  setPageInfo({ limit: pageSize, offset: arrayConnectionString((page - 1) * pageSize) });
};

export const onChangeOrderBy = (columns, setOrderBy, refetch) => {
  setOrderBy({ orderBy: columns.join(',') });
  refetch();
};
