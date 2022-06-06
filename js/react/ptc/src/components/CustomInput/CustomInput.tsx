import React from 'react';
import { Input } from 'antd';
import { InputProps } from 'antd/lib/input';
import { Icon, IconName } from '../Icon/Icon';
import { colorSvgDefault } from '../Styles/Colors';
import styled from 'styled-components'

type CustomInputProps = {
  className?: string;
  label?: string;
  width?: number;
  name?: string;
  iconName?: IconName;
} & InputProps;

function CustomInput({
  className = '',
  label,
  width,
  name = 'custom-input',
  iconName = 'transfer',
  ...props
}: CustomInputProps): React.ReactElement {
  return (
    <StyledCustomInputDiv className={className}>
      {label && (
        <label className="custom-input_label" htmlFor={name}>
          {label}
        </label>
      )}
      <Input
        size="large"
        style={{ width: width ? `${width}px` : '100%' }}
        prefix={<Icon name={iconName} size={18} color={colorSvgDefault} />}
        name={name}
        allowClear={true}
        {...props}
      />
    </StyledCustomInputDiv>
  );
}

const StyledCustomInputDiv = styled.div`
  .custom-input_label {
    color: #1A1A1A;
    font-size: 14px;
    font-weight: bold;
    height: 32px;
    line-height: 32px;
    margin: 0;
  }
  .ant-input {
    &-affix-wrapper {
      padding: 6.5px 10px 6.5px 6px;
      border-radius: 4px;
      input {
        text-overflow: unset;
      }
    }
    &-prefix {
      margin-right: 9px;
    }
  }
`;


export type { CustomInputProps };
export { CustomInput };
