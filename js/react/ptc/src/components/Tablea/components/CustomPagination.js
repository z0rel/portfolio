import styled from 'styled-components';
import { Pagination, Select } from 'antd';
import React from 'react';

const StyledPaginationAncor = styled.button`
  color: #2c5de5 !important;
  border: none;
  background: none;
  -moz-appearance: none;
  -webkit-appearance: none;
  appearance: none;
  &:disabled {
    color: rgba(0, 0, 0, 0.85) !important;
    cursor: default;
  }
`;

export const CustomPagination = ({ total, current, pageSize, disabled, onChange, showItems, onChangePageSize }) => {
  const { Option } = Select;

  function itemRender(current, type, originalElement) {
    if (type === 'prev') {
      return <StyledPaginationAncor>&lt; Назад</StyledPaginationAncor>;
    }
    if (type === 'next') {
      return <StyledPaginationAncor>Вперед &gt;</StyledPaginationAncor>;
    }
    return originalElement;
  }

  total = total ?? 1;
  current = current ?? 1;

  let lastPage = current - 1;
  let startShowItems = lastPage * pageSize + 1;
  let endShowItems = lastPage * pageSize + showItems;

  function handleChangePageSize(value) {
    onChangePageSize(current, value);
  }

  return (
    <StyledCustomPagination>
      <div className={'page-size-changer-and-show-total'}>
        <StyledPageSizeChanger>
          Отображать по:
          <StyledSelect defaultValue={pageSize} onChange={handleChangePageSize}>
            {['10', '15', '25', '50', '100', '1000'].map((ps, i) => {
              return (
                <Option key={ps} value={ps}>
                  {ps}
                </Option>
              );
            })}
          </StyledSelect>
        </StyledPageSizeChanger>
        {endShowItems > 0 && (
          <StyledShowTotal>
            <span>|</span>Показано с {startShowItems} до {endShowItems > total ? total : endShowItems} из {total}
          </StyledShowTotal>
        )}
      </div>
      <StyledPagination
        disabled={disabled}
        onChange={onChange}
        total={total}
        current={current}
        pageSize={pageSize}
        itemRender={itemRender}
        showSizeChanger={false}
      />
    </StyledCustomPagination>
  );
};

const StyledPageSizeChanger = styled.div``;

const StyledShowTotal = styled.div`
  span {
    padding: 0px 20px;
  }

  @media all and (max-width: 1024px) {
    span {
      display: none;
    }
  }
`;

const StyledPagination = styled(Pagination)`
  .ant-pagination-prev.ant-pagination-disabled a:not([href]):not([class]) {
    color: rgba(0, 0, 0, 0.85);
  }

  .ant-pagination-prev a:not([href]):not([class]),
  .ant-pagination-next a:not([href]):not([class]) {
    color: #2c5de5;
  }

  .ant-pagination-item {
    border: 0px;
  }

  .ant-pagination-item:hover {
    border-radius: 4px;
    border: 1px solid #2c5de5;
  }

  .ant-pagination-item-active {
    font-weight: 500;
    color: #fff;
    background: #2c5de5;
    border-color: #2c5de5;
    border-radius: 4px;
  }

  .ant-select:not(.ant-select-customize-input) .ant-select-selector {
    border-radius: 4px;
  }

  .ant-select:not(.ant-select-customize-input) .ant-select-selector:hover {
    border: 1px solid #2c5de5;
  }
`;

const StyledSelect = styled(Select)`
  margin-left: 10px;
  width: 80px;

  .ant-select-selector {
    border-radius: 4px !important;
  }

  .ant-select-selector:hover {
    border: 1px solid #2c5de5 !important;
  }
`;

const StyledCustomPagination = styled.div`
  display: flex;
  justify-content: space-between;
  border: 1px solid #d3dff0;
  border-radius: 0px 0px 8px 8px;
  margin-bottom: 30px;
  border-top: 0px;
  padding: 10px;
  line-height: 32px;
  font-family: Arial, Helvetica, sans-serif;
  outline: 0;
  font-size: 14px;

  .page-size-changer-and-show-total {
    display: flex;
  }

  @media all and (max-width: 1024px) {
    .page-size-changer-and-show-total {
      display: block;
    }
  }
`;
