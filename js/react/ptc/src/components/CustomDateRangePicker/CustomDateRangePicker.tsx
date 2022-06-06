import React from 'react';
import { DatePicker } from 'antd';
import { RangePickerProps } from 'antd/lib/date-picker';
import styled from 'styled-components';
import 'moment/locale/ru';
import locale from 'antd/es/date-picker/locale/ru_RU';

import { ReactComponent as DateIcon } from '../../img/left-bar/filter/date.svg';
import { DROPDOWN_TOP_ALIGN } from '../Form/dropdownPlacements';

type CustomDateRangePickerProps = {
  className?: string;
  label?: string;
  name?: string;
  svgIcon?: React.ReactNode;
  width?: number;
} & RangePickerProps;

function CustomDateRangePicker({
  className = '',
  label,
  name = 'custom-datepicker',
  svgIcon,
  width,
  ...props
}: CustomDateRangePickerProps): React.ReactElement {
  return (
    <StyledCustomDateRangePickerDivWrapper className={className}>
      {label && (
        <label className="custom-date-range-picker_label" htmlFor={name}>
          {label}
        </label>
      )}
      <DatePicker.RangePicker
        id={name}
        size="large"
        style={{ width: width ? `${width}px` : '100%' }}
        suffixIcon={svgIcon || <DateIcon />}
        // по дизайну placeholder должен быть одной строкой, данный компонент не позволяет это реализовать
        dropdownAlign={props.dropdownAlign || DROPDOWN_TOP_ALIGN}
        placeholder={props.placeholder || ['Начало', 'Окончание']}
        locale={locale}
        {...props}
      />
    </StyledCustomDateRangePickerDivWrapper>
  );
}

const StyledCustomDateRangePickerDivWrapper = styled.div`
  &_label {
    color: #1a1a1a;
    font-size: 14px;
    font-weight: bold;
    height: 32px;
    line-height: 32px;
    margin: 0;
  }
  .ant-picker {
    position: relative;
    border-radius: 4px;
    padding: 6.5px 11px 6.5px 8px;
    &-suffix {
      order: -1;
      margin: 0 9px 0 0;
    }
    &-active-bar {
      margin-left: 32px;
    }
    &-separator {
      line-height: 0;
    }
  }
`;

export type { CustomDateRangePickerProps };
export { CustomDateRangePicker };
