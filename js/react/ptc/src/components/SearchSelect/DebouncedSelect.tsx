import * as React from 'react';
import { useEffect, useState } from 'react';
import useDebounce from '../../containers/Administration/components/useDebounce';
import { useLazyQuery } from '@apollo/client';
import { DebouncedSelectProps, GetQueryVariablesType } from './utils/utils';
import { IconedSelect } from './IconedSelect';
import './style.scss';

export interface IsMounted {
  current: boolean;
}

export const DebouncedSelect = ({
                                  name,
                                  query,
                                  defaultOpen = undefined,
                                  dataPredicate = undefined,
                                  dataUnpack = undefined,
                                  dataUnpackSpec = undefined,
                                  emptyrowTitle = undefined,
                                  placeholderSpec = undefined,
                                  getQueryVariables = undefined,
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
                                  refetchOnChangeThisVar = undefined,
                                  valueSpec = undefined,
                                  preloadedKeys = undefined,
                                  additionalVars = undefined,
                                  ...props
                                }: DebouncedSelectProps): React.ReactElement => {
  const [value, setValue] = useState(undefined);
  const [searchText, setSearchText] = useState<string>(value!);

  const debouncedSearchTerm = useDebounce(searchText, 500);
  const [getQueryData, { loading, data }] = useLazyQuery(query);

  if (getQueryVariables === undefined)
    getQueryVariables = (term: string) => ({ title_Icontains: term });

  useEffect(() => {
    if (componentIsMounted === undefined || componentIsMounted?.current)
      getQueryData({ variables: {...(getQueryVariables as GetQueryVariablesType)(debouncedSearchTerm), ...additionalVars} });
  }, [debouncedSearchTerm, getQueryData, loading, refetchOnChangeThisVar, componentIsMounted, getQueryVariables, additionalVars]);

  return (
    <IconedSelect
      name={name}
      value={value}
      setValue={setValue}
      valueSpec={valueSpec}
      dataPredicate={dataPredicate}
      dataUnpack={dataUnpack}
      dataUnpackSpec={dataUnpackSpec}
      emptyrowTitle={emptyrowTitle}
      placeholderSpec={placeholderSpec}
      dropdownAlignTop={dropdownAlignTop}
      dropdownAlignBottom={dropdownAlignBottom}
      dropdownAlignDefault={dropdownAlignDefault}
      valueSelector={valueSelector}
      formitem={formitem}
      rules={rules}
      label={label}
      loading={loading}
      disabled={disabled}
      componentIsMounted={componentIsMounted}
      nodeToTitle={nodeToTitle}
      hasInputIcon={hasInputIcon}
      size={size}
      style={style}
      onClear={onClear}
      handleValueChanged={handleValueChanged}
      queryKey={queryKey}
      data={data}
      setSearchText={setSearchText}
      defaultOpen={defaultOpen}
      preloadedKeys={preloadedKeys}
      {...props}
    />
  );
};
