import { StyledPenGreen, StyledTrash } from '../../../containers/Administration/components/Styled';
import React from 'react';

const filterColumns = (columns) => {
  return columns
    .filter((col, index) => col.isShowed !== false || col.hasOwnProperty('children') || col.dataIndex === 'btn-remove')
    .map((col) => (!col.hasOwnProperty('children') ? col : { ...col, children: filterColumns(col.children) }))
    .filter((col, index) => !col.hasOwnProperty('children') || col.children.length > 0);
};

export const getLocalColumns = ({
  columns,
  setColumnsFilter,
  edit,
  setOpenEditModal,
  setEditingItem,
  del,
  openDeleteModal,
  choosedBlock,
}) => {
  const localColumns = filterColumns(columns);

  if (edit) {
    localColumns.push({
      width: 50,
      render: (text, record) => {
        return (
          <StyledPenGreen
            onClick={() => {
              setOpenEditModal && setOpenEditModal(true);
              setEditingItem && setEditingItem(record);
            }}
          />
        );
      },
    });
  }
  if (del || (edit && del === null)) {
    localColumns.push({
      width: 50,
      render: (text, record) => {
        return (
          <StyledTrash
            className="EditTrashStyle"
            alt=""
            onClick={() => openDeleteModal && openDeleteModal(record, choosedBlock)}
          />
        );
      },
    });
  }
  return localColumns;
};
