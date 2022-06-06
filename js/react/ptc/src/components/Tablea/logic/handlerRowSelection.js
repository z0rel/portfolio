export const handlerRowSelection = ({
                                      setSelectedItems,
                                      selectedItems,
                                      choosedBlock,
                                      selectAll,
                                      setSelectAll,
                                      selectedRowKeys,
                                    }) => ({
  type: 'checkbox',
  onChange: (selectedRowKeys, selectedRows) => {
    // console.log(selectedRowKeys)
    // this.props.setConstructionsIdSet(selectedRowKeys);
    setSelectedItems &&
    setSelectedItems({
      ...selectedItems,
      [choosedBlock]: {
        all: false,
        rows: selectedRows.filter((item) => (item.block ? item.block === choosedBlock : true)),
        keys: selectedRows
          .filter((item) => (item.block ? item.block === choosedBlock : true))
          .map((item) => item.key),
      },
    });
  },
  getCheckboxProps: (record) => ({
    disabled: record.name === 'Disabled User',
    name: record.name,
  }),
  onSelectAll: (selected, selectedRows, changeRows) => {
    let prev = selectAll[choosedBlock];
    if (prev === undefined || prev === null)
      prev = false;
    setSelectAll({ ...selectAll, [choosedBlock]: !prev });
    setSelectedItems &&
    setSelectedItems({
      ...selectedItems,
      [choosedBlock]: {
        all: !prev,
        rows: selectedRows.filter((item) => (item.block ? item.block === choosedBlock : true)),
        keys: selectedRows
          .filter((item) => (item.block ? item.block === choosedBlock : true))
          .map((item) => item.key),
      },
    });
  },
  selectedRowKeys: selectAll[choosedBlock] ? selectedRowKeys : undefined,
  preserveSelectedRowKeys: true,
});
