import React from 'react';
import { DebouncedSelect } from '../SearchSelect/DebouncedSelect';
import { getPropsByType } from './selectConfig';

import './CustomDebouncedSelect.scss';

// type TypeProps = keyof typeof SelectTypeName;
// type CustomDebouncedSelectProps = {
//   // мы можем передать одно из предустановленных имён для получения нужных данных
//   type: TypeProps;
//   name?: string;
//   rules?: any[] | undefined;
//   label?: string;
//   placeholder?: string;
// };

function CustomDebouncedSelect({
  name,
  rules = [],
  label,
  placeholder = '',
  type,
  ...otherProps
}) {
  const props = getPropsByType(type);

  return (
    <div className="custom-debounced-select">
      <DebouncedSelect
        {...props}
        rules={rules}
        label={label || props.label}
        placeholderSpec={{ ...props.placeholderSpec, title: placeholder || props.placeholderSpec.title }}
        name={name}
        {...otherProps}
      />
    </div>
  );
}

export { CustomDebouncedSelect };
