import React, { CSSProperties } from 'react';
import { Form } from 'antd';
import { NamePath, Rule } from 'rc-field-form/lib/interface';
import { FormLabelAlign } from 'antd/lib/form/interface';
import styled from 'styled-components';

export interface RulesInterface {
  message?: string | React.ReactElement;
  required?: boolean;
  rules?: Rule[];
}

interface FormItemProps {
  rules?: RulesInterface | Rule[];
  name?: NamePath;
  labelAlign?: FormLabelAlign;
  children: React.ReactChildren | React.ReactNode;
  marginTop?: string | number;
  className?: string;
  initialValue?: any;
  label?: React.ReactNode /** Метка, которая будет помещена в span */
  ;
  labelWrapper?: React.ReactNode;
  required?: boolean;
  dependencies?: NamePath[];
}

export const FormItem = ({
                           name,
                           rules = undefined,
                           children,
                           labelAlign = 'left',
                           marginTop = undefined,
                           initialValue = undefined,
                           className = undefined,
                           label = undefined,
                           labelWrapper = undefined,
                           required = undefined,
                           dependencies = undefined,
                           ...props
                         }: FormItemProps): React.ReactElement => {
  let styleObj: CSSProperties | undefined = undefined;
  if (marginTop)
    styleObj = { marginTop: marginTop };

  if (labelWrapper === undefined && label)
    labelWrapper = <StyledLabelSpan style={styleObj}>{label}</StyledLabelSpan>;

  let rulesObj: Rule[] | undefined = undefined;
  if (rules) {
    if (typeof rules === 'object' && rules !== null && rules instanceof Array)
      rulesObj = rules;
    else if (rules?.rules)
      rulesObj = rules.rules;
    else if (rules?.required)
      rulesObj = [{ required: rules.required, message: rules.message }];
  }

  return (
    <StyledFormItem
      className={'editForm-item' + (className ? ' ' + className : '')}
      labelAlign={labelAlign}
      colon={false}
      name={name}
      rules={rulesObj}
      label={labelWrapper}
      initialValue={initialValue}
      required={required}
      dependencies={dependencies}
      {...props}
    >
      {children}
    </StyledFormItem>
  );
};

const StyledFormItem = styled(Form.Item)`
  display: flex;
  flex-direction: column;
  margin-right: 0 !important;
  width: 100%;
`;

const StyledLabelSpan = styled.span`
  color: #1a1a1a;
  font-size: 14px;
  font-weight: bold;
`;
