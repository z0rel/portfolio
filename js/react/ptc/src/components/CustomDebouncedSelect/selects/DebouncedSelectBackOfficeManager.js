import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import React from 'react';
import { QUERY_MANAGERS } from '../../../containers/Installations/Projects/queries';
import { getManagerName, managerSelectFilter } from '../../Logic/getManager';
import { ReactComponent as IconCreator } from '../../../img/left-bar/filter/creator.svg';

export const DebouncedSelectBackOfficeManager = ({formitem=StyledFormItem, name='backOfficeManagerId', ...props}) => {
  return <DebouncedSelect
    dropdownAlignBottom
    name={name}
    label="Менеджер бэк-офиса"
    formitem={{ formitem: formitem }}
    query={QUERY_MANAGERS}
    getQueryVariables={managerSelectFilter}
    placeholderSpec={{
      svg: IconCreator,
      title: 'Менеджер бэк-офиса',
      svgMarginTop: '2px',
      needSvgInDropdown: true,
      titleMarginLeft: '-.5rem',
    }}
    valueSelector={(node) => node?.id}
    queryKey="searchUser"
    dataUnpackSpec={{ unpackForLocalCompare: getManagerName }}
    {...props}
  />;
}
