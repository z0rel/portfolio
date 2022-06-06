import React, { useContext } from 'react';
import styled from 'styled-components'
import { EstimateContext } from '../Estimate';
import { DatePicker } from 'antd';

export const DropdownPeriodFilter = (props) => {
  const { periodFilter, setPeriodFilter } = useContext(EstimateContext);

  return (
    <div style={{ width: '260px' }}>
      <div style={{ padding: '16px' }}>
        <StyledPSortIncrease
          style={{ color: periodFilter === 'increase' ? '#2C5DE5' : '#1A1A1A' }}
          onClick={() => {
            setPeriodFilter((prevState) => {
              switch (prevState) {
                case 'increase':
                  return '';
                default:
                  return 'increase';
              }
            });
            props.clearFilters();
            props.confirm();
          }}>
          Сортировать по увеличению
        </StyledPSortIncrease>
        <StyledPSortDecrease
          style={{ color: periodFilter === 'decrease' ? '#2C5DE5' : '#1A1A1A' }}
          onClick={() => {
            props.confirm();
            props.clearFilters();
            setPeriodFilter((prevState) => {
              switch (prevState) {
                case 'decrease':
                  return '';
                default:
                  return 'decrease';
              }
            });
          }}>
          Сортировать по уменьшению
        </StyledPSortDecrease>
      </div>
      <div style={{ borderTop: '1px solid #D3DFF0' }}>
        <div style={{ padding: '16px' }}>
          <StyledPOptions>
            ОПЦИИ
          </StyledPOptions>
          <DatePicker style={{ width: '100%' }}
            onChange={(val) => {
              if (!val) {
                props.clearFilters();
              }
            }}
            format="DD.MM.YYYY"
            onSelect={(val) => {
              props.setSelectedKeys([val.toDate().setHours(0, 0, 0, 0)]);
              props.confirm();
              console.log('cleared');
            }}
            placeholder="Выберите дату"
          />
        </div>
      </div>
    </div>
  );
};

const StyledPOptions = styled.p`
    color: #656565;
    font-size: 12px;
`;

const StyledPSortIncrease = styled.p`
    font-size: 14px;
    margin-bottom: 16px;
    cursor: pointer;
`;
const StyledPSortDecrease = styled.p`
    font-size: 14px;
    margin-bottom: 0;
    cursor: pointer;
`;

