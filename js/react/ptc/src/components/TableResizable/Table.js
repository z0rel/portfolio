import React, { Component } from 'react';
import { Table } from 'antd';
import styled from 'styled-components';
import { Resizable } from 'react-resizable';
import PropTypes from 'prop-types';

import icon_pen from '../../img/outdoor_furniture/table_icons/bx-dots-vertical.svg';
import icon_delete from '../../img/outdoor_furniture/red_can.svg';

const ResizableTitle = (props) => {
  const { onResize, width, ...restProps } = props;

  if (!width) {
    return <th {...restProps} />;
  }
  return (
    <Resizable
      width={width}
      height={0}
      handle={
        <span
          className="react-resizable-handle"
          onClick={(e) => {
            e.stopPropagation();
          }}
        />
      }
      onResize={onResize}
      draggableOpts={{ enableUserSelectHack: false }}
    >
      <th {...restProps} />
    </Resizable>
  );
};

export default class AdvertisingParties extends Component {
  state = {
    columns: this.props.columns,
  };

  components = {
    header: {
      cell: ResizableTitle,
    },
  };
  rowSelection = {
    onChange: () => {
    },
    getCheckboxProps: (record) => ({
      disabled: record.name === 'Disabled User', // Column configuration not to be checked
      name: record.name,
    }),
  };

  handleResize = (index) => (e, { size }) => {
    this.setState(({ columns }) => {
      const nextColumns = [...columns];

      const delta = columns[index].width - size.width;

      nextColumns[index + 1].width += delta;

      nextColumns[index] = {
        ...nextColumns[index],
        width: size.width,
      };
      return { columns: nextColumns };
    });
  };

  render() {
    const { loading, footer } = this.props;
    const columns = this.props.columns.map((col, index) => ({
      ...col,
      onHeaderCell: (column) => ({
        width: column.width,
        onResize: this.handleResize(index),
      }),
    }));

    this.props.edit &&
    columns.push(
      {
        width: 50,
        render: (text, record) => {
          return (
            <img
              alt="edit icon"
              style={{ cursor: 'pointer' }}
              onClick={() => {
                this.props.setOpenEditModal(true);
                this.props.setEditingItem(record);
              }}
              src={icon_pen}
            />
          );
        },
      },
      {
        width: 50,
        render: (text, record) => {
          return (
            <img
              alt="delete icon"
              style={{ cursor: 'pointer' }}
              onClick={() => this.props.openModal(record, this.props.deleteEstimate, this.props.setDeleted)}
              src={icon_delete}
            />
          );
        },
      },
    );
    return (
      <StyledTable
        rowSelection={
          this.props.select
            ? {
              ...this.rowSelection,
            }
            : null
        }
        loading={loading}
        footer={footer ? () => footer : undefined}
        components={this.components}
        columns={columns}
        dataSource={this.props.data}
        pagination={true}
      />
    );
  }
}

AdvertisingParties.propTypes = {
  loading: PropTypes.bool,
  footer: PropTypes.string,
};
AdvertisingParties.defaultProps = {
  loading: false,
  footer: undefined,
};

const StyledTable = styled(Table)`
  width: 100%;

  .ant-table-thead > tr > th {
    background: #fff;
  }

  .ant-table table {
    border-collapse: separate !important;
  }

  & .ant-table {
    border: 1px solid #d3dff0 !important;
    border-radius: 8px;
  }

  & .ant-table-thead > tr > th {
    background: transparent;
    color: #1a1a1a;
    font-weight: 600;
    border-bottom: 1px solid #d3dff0 !important;
  }

  & .ant-table-thead > tr > th[colspan]:not([colspan='1']) {
    left: 5px;
  }

  & .ant-table-thead > tr > th:not(:last-child) > span {
    background: #d3dff0;
    width: 1px;
  }

  & .ant-table-tbody > tr.ant-table-row:hover > td {
    background: #d3dff0;
  }

  & .ant-table-tbody tr:nth-child(even) {
    background: #f5f7fa !important;
  }

  & .ant-table-tbody tr:nth-child(odd) {
    background: #fff;
  }

  svg {
    vertical-align: unset;
  }

  .ant-table-content {
    overflow: auto;
  }
  margin-bottom: 190px;
  th,
  td {
    color: #1a1a1a !important;
    font-weight: 600 !important;
  }

  .ant-table {
    border: 1px solid #d3dff0 !important;
    border-radius: 8px;
  }

  .ant-table-row td:not(:first-child) {
    padding: 5px !important;
  }

  th {
    background: transparent !important;
  }

  .ant-table-body tr td:not(:first-child) {
    background: #f5f7fa !important;
  }
`;
