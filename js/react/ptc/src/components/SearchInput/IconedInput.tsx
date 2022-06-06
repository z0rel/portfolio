import * as React from 'react';
import { useState } from 'react';
import { Rule } from 'rc-field-form/lib/interface';

import { getDropdownAlign, getFormItem, getPlaceholder, getSuffix, IconedSelectProps, mapDataToOptions } from './utils';

import { StyledSelectImgPlaceholder, StyledSelectNoImgPlaceholder } from './StyledInputImg';

type OptionType = typeof StyledSelectImgPlaceholder.Option | Array<typeof StyledSelectImgPlaceholder.Option>;

export const IconedInput = ({
                              name,
                              dataPredicate,
                              dataUnpack = undefined,
                              dataUnpackSpec = undefined,
                              emptyrowTitle = undefined,
                              placeholderSpec = undefined,
                              dropdownAlignTop = false,
                              dropdownAlignBottom = false,
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
                              defaultOpen = undefined,
                              ...props
                            }: IconedSelectProps): React.ReactElement => {
  const [selectOpened, setSelectOpened] = useState(false);

  const renderedNodes: any = new Map();

  const StyledSelectComponent: any =
    hasInputIcon || (placeholderSpec && (placeholderSpec.image || placeholderSpec.svg))
      ? StyledSelectImgPlaceholder
      : StyledSelectNoImgPlaceholder;

  const options: any = mapDataToOptions({
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
  });

  const suffix = getSuffix(selectOpened, placeholderSpec);

  const mountedOk = componentIsMounted === undefined || componentIsMounted?.current;

  const styledSelectComponent = (
    <StyledSelectComponent
      // open={selectOpened}
      allowClear
      style={style}
      dropdownAlign={getDropdownAlign(dropdownAlignTop, dropdownAlignBottom)}
      placeholder={getPlaceholder(placeholderSpec)}
      suffixIcon={suffix}
      size={size || 'large'}
      showSearch
      value={!loading && data ? value : undefined}
      defaultActiveFirstOption={false}
      showArrow={true}
      filterOption={false}
      notFoundContent={null}
      loading={loading}
      disabled={disabled}
      defaultOpen={defaultOpen}
      onSearch={(value: any) => {
        mountedOk && setSearchText && setSearchText(value);
      }}
      onChange={(value: any, option: OptionType) => {
        mountedOk && setValue && setValue(value);
        mountedOk && handleValueChanged && handleValueChanged(value, renderedNodes);
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
