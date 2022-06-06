import { FormItem } from '../../../../components/Form/FormItem';
import { InputNumber } from 'antd';
import React from 'react';
import { NamePath, Rule } from 'rc-field-form/lib/interface';
import { nullFormatterPercent, nullFormatterPrice } from './utils';

const discountValidator = (fieldName: string, message: string): Rule => (form) => ({
  validator: (rule, value) => {
    let other = form.getFieldValue(fieldName);
    if (other === undefined)
      other = null;
    if (value === undefined)
      value = null;
    if (
      (other === null && value === null) ||
      (other === null && value !== null) ||
      (other !== null && value === null)
    ) {
      return Promise.resolve();
    }
    return Promise.reject(message);
  },
});

interface FormItemInputDependProps {
  name?: NamePath;
  label?: React.ReactNode /** Метка, которая будет помещена в span */
  ;
  dependField: string;
  msg: string;
  formatter?: (value: number | string | undefined) => string;
  className?: string;
}

export const FormItemInputDepend = ({
                                      name,
                                      label,
                                      dependField,
                                      msg,
                                      formatter,
                                      ...props
                                    }: FormItemInputDependProps): React.ReactElement => {
  return (
    <FormItem
      name={name}
      label={label}
      initialValue={null}
      required={false}
      rules={[discountValidator(dependField, msg)]}
      dependencies={[dependField]}
      {...props}
    >
      <InputNumber style={{ width: '100%' }} size="large" formatter={formatter}/>
    </FormItem>
  );
};

interface FormItemPercentValueProps {
  name1: string;
  label1: string;
  name2: string;
  label2: string;
  className1?: string;
  className2?: string;
}

export const FormItemPercentValue = ({
                                       name1,
                                       label1,
                                       name2,
                                       label2,
                                       className1 = undefined,
                                       className2 = undefined,
                                       ...props
                                     }: FormItemPercentValueProps): React.ReactElement => {
  const msg = `"${label1}" и "${label2}" не могут быть заданы одновременно`;
  return (
    <>
      <FormItemInputDepend
        name={name1}
        label={label1}
        formatter={nullFormatterPercent}
        dependField={name2}
        msg={msg}
        className={className1}
        {...props}
      />
      <FormItemInputDepend
        name={name2}
        label={label2}
        formatter={nullFormatterPrice}
        dependField={name1}
        msg={msg}
        className={className2}
        {...props}
      />
    </>
  );
};
