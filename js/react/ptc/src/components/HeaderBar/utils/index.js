export const changeColumns = (
  dataIndex, columnsForPopup,
  setColumnsForPopup, setColumnsTable
) => {
  let localColumnsForPopup = columnsForPopup.map(col => {
    if(col.children) {
      col.children = col.children.map(item => {
        if(item.dataIndex  && item.dataIndex === dataIndex) {
          item.isShowed = !item.isShowed;
        }
        return item
      });
    }
    if(col.dataIndex && col.dataIndex === dataIndex) {
      col.isShowed = !col.isShowed;
    }
    return col
  })

  setColumnsForPopup(localColumnsForPopup);

  let filterColumnTables = (columnsTable) => {
    let filterPredicate = _item => {
      return (
        (_item.children && _item.children.length > 0) ||
        (_item.children === undefined && (_item.isShowed || _item.dataIndex === 'btn-remove'))
    )
    };
    return columnsTable.filter(filterPredicate).map(_item => {
      let item = { ..._item };
      if (item.children) {
        item.children = item.children.filter(el => el.isShowed ? el : false);
      }
      return item;
    }).filter(filterPredicate);
  }

  setColumnsTable([...filterColumnTables(localColumnsForPopup)]);
};
