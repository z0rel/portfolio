import * as React from 'react';
import { useState } from 'react';
import { Rule } from 'rc-field-form/lib/interface';

import { getDropdownAlign, getFormItem, getPlaceholder, getSuffix, IconedSelectProps } from './utils/utils';
import { mapDataToOptions } from './utils/mapDataToOptions';

import { StyledSelectImgPlaceholder, StyledSelectNoImgPlaceholder } from './StyledSelectImg';

type OptionType = typeof StyledSelectImgPlaceholder.Option | Array<typeof StyledSelectImgPlaceholder.Option>;

interface OptionInterface {
  key: string;
  value: string;
  children: any;
  title: string;
  className: string;
}

export const IconedSelect = ({
                               name,
                               dataPredicate,
                               dataUnpack = undefined,
                               dataUnpackSpec = undefined,
                               emptyrowTitle = undefined,
                               placeholderSpec = undefined,
                               dropdownAlignTop = false,
                               dropdownAlignBottom = false,
                               dropdownAlignDefault = false,
                               valueSelector = undefined,
                               formitem = { antd: true },
                               rules = undefined,
                               label = undefined,
                               disabled = false,
                               componentIsMounted = undefined,
                               nodeToTitle = undefined,
                               hasInputIcon = false,
                               size = undefined,
                               style = undefined,
                               onClear = undefined,
                               handleValueChanged = undefined,
                               queryKey = undefined, // Ключ запроса (вместо параметров dataUnpack и dataPredicate)
                               setSearchText = undefined,
                               value = undefined,
                               setValue = undefined,
                               valueSpec = undefined,
                               loading = undefined,
                               data = undefined,
                               mode = undefined,
                               defaultOpen = undefined,
                               preloadedKeys = undefined,
                               ...props
                             }: IconedSelectProps): React.ReactElement => {
  const [selectOpened, setSelectOpened] = useState(false);

  const isTagMode = mode === 'tags';
  const renderedNodes: any = new Map();

  const StyledSelectComponent: any =
    hasInputIcon || (placeholderSpec && (placeholderSpec.image || placeholderSpec.svg))
      ? StyledSelectImgPlaceholder
      : StyledSelectNoImgPlaceholder;

  const options: any = mapDataToOptions({
    name,
    loading,
    data,
    Option: StyledSelectComponent.Option,
    renderedNodes,
    dataPredicate,
    dataUnpack,
    dataUnpackSpec,
    queryKey,
    emptyrowTitle,
    valueSelector,
    placeholderSpec,
    nodeToTitle,
    isTagMode,
    preloadedKeys,
  });

  const suffix = getSuffix(selectOpened, placeholderSpec);

  if (isTagMode && value) {
    style = { ...style, height: `${2.577 * ((value.length as number) + 1)}rem` };
  }
  const mountedOk = componentIsMounted === undefined || componentIsMounted?.current;

  const styledSelectComponent = (
    <StyledSelectComponent
      open={selectOpened}
      showSearch
      allowClear
      style={style}
      dropdownAlign={getDropdownAlign(dropdownAlignTop, dropdownAlignBottom, dropdownAlignDefault)}
      placeholder={getPlaceholder(placeholderSpec)}
      suffixIcon={suffix}
      size={size || 'large'}
      value={!loading && data ? value : undefined}
      optionFilterProp="title"
      optionLabelProp="title"
      defaultActiveFirstOption={false}
      showArrow={true}
      filterOption={false}
      notFoundContent={null}
      loading={loading}
      disabled={disabled}
      mode={mode}
      defaultOpen={defaultOpen}
      onSearch={(value: any) => {
        mountedOk && setSearchText && setSearchText(value);
      }}
      onChange={(value: any, option: OptionType) => {
        mountedOk && setValue && setValue(value);
        mountedOk && handleValueChanged && handleValueChanged(value, option);
      }}
      onDropdownVisibleChange={(opened: boolean) => {
        mountedOk && setSelectOpened(opened);
      }}
      onClear={onClear}
    >
      {options}
    </StyledSelectComponent>
  );
  if (formitem.childsOnly) {
    return styledSelectComponent;
  }
  else {
    const FormitemReactNode: any = getFormItem(formitem);
    return (
      <FormitemReactNode name={name} rules={rules as Rule[] | undefined} label={label} {...props}>
        {styledSelectComponent}
      </FormitemReactNode>
    );
  }
};
