import * as React from 'react';
import { Moment } from 'moment';
import { DatePicker as AntDatePicker } from 'antd';
import { DatePickerProps } from 'antd/lib/date-picker';
import { Icon, IconName } from '../Icon/Icon';
import { colorSvgDefault } from '../Styles/Colors';
import styled from 'styled-components';
import 'moment/locale/ru';
import locale from 'antd/es/date-picker/locale/ru_RU';

type CustomDatePickerProps = {
  className?: string;
  disabled?: boolean;
  placeholder?: string;
  width?: number;
  label?: string;
  name?: string;
  value?: Moment;
  iconName?: IconName;
} & DatePickerProps;

function CustomDatePicker({
  className = '',
  disabled,
  placeholder,
  width,
  label,
  name = 'custom-datepicker',
  value,
  iconName = 'transfer',
  ...props
}: CustomDatePickerProps): React.ReactElement {
  return (
    <StyledCustomDatePicker className={`ant-col ${className}`}>
      {label && (
        <label className="custom-date-picker_label" htmlFor={name}>
          {label}
        </label>
      )}
      <AntDatePicker
        locale={locale}
        placeholder={placeholder}
        size="large"
        disabled={disabled}
        style={{ width: width ? `${width}px` : '100%' }}
        suffixIcon={<Icon name={iconName} size={18} color={colorSvgDefault} />}
        id={name}
        {...props}
      />
    </StyledCustomDatePicker>
  );
}

const StyledCustomDatePicker = styled.div`
  &_label {
    color: #1a1a1a;
    font-size: 14px;
    font-weight: bold;
    height: 32px;
    line-height: 32px;
    margin: 0;
  }
  .ant-picker-input {
    flex-direction: row-reverse;
  }
  .ant-picker-suffix {
    margin: 0 9px 0 0;
  }
`;

export type { CustomDatePickerProps };
export { CustomDatePicker };
