const addSortColumns = (sort, sortColumns, paginationRelayApi) => {
  if (sort.order) {
    let order = sort.order === 'ascend';
    let sign = paginationRelayApi ? '-' : '_';
    if (sort.column && sort.column.orderByFields) {
      for (let orderByField of sort.column.orderByFields) {
        sortColumns.push(order ? orderByField : `${sign}${orderByField}`);
      }
    }
    else {
      sortColumns.push(order ? sort.field : `${sign}${sort.field}`);
    }
  }
};

export const getSortColumns = (sorter, paginationRelayApi) => {
  if (sorter) {
    let sortColumns = [];
    if (Array.isArray(sorter)) {
      sorter.forEach((sort, i) => {
        addSortColumns(sort, sortColumns, paginationRelayApi);
      });
    }
    else {
      addSortColumns(sorter, sortColumns, paginationRelayApi);
    }
    return sortColumns;
  }
  return [];
};

export const getFrontendSortedData = (sorter, dataToSorting) => {
  if (!sorter || sorter?.length === 0) {
    return dataToSorting;
  }
  let copiedDataToSorting = [...dataToSorting];
  let copiedSorter = Array.isArray(sorter) ? sorter : [sorter];
  copiedDataToSorting.sort((l, r) => {
    for (let sorterObj of copiedSorter) {
      if (sorterObj && sorterObj.column && sorterObj.column.sorter) {
        const cmpValue = sorterObj.column.sorter.compare(l, r);
        if (cmpValue !== 0) {
          return cmpValue;
        }
      }
    }
    return 0;
  });
  return copiedDataToSorting;
};
