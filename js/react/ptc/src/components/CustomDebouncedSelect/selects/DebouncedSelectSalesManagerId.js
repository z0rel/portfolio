import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import { QUERY_MANAGERS } from '../../../containers/Installations/Projects/queries';
import { getManagerName, managerSelectFilter } from '../../Logic/getManager';
import React from 'react';
import { ReactComponent as IconManager } from '../../../img/left-bar/filter/manager.svg';

export const DebouncedSelectSalesManagerId = ({
  formitem = StyledFormItem,
  customFormItem = undefined,
  name = 'salesManagerId',
  label = 'Менеджер по продажам',
  ...props
}) => {
  if (props.dropdownAlignBottom === undefined && props.dropdownAlignTop === undefined) {
    props.dropdownAlignBottom = true;
  }

  return (
    <DebouncedSelect
      name={name}
      label={label}
      formitem={customFormItem ? customFormItem : { formitem: formitem }}
      query={QUERY_MANAGERS}
      getQueryVariables={managerSelectFilter}
      placeholderSpec={{
        svg: IconManager,
        title: 'Менеджер по продажам',
        svgMarginTop: 0,
        needSvgInDropdown: true,
        titleMarginLeft: '-.5rem',
      }}
      valueSelector={(node) => node?.id}
      queryKey="searchUser"
      dataUnpackSpec={{ unpackForLocalCompare: getManagerName }}
      {...props}
    />
  );
};
