import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import React from 'react';
import { QUERY_PROJECTS } from '../../../containers/Installations/Orders/queries';
import getProjectSelectFilter from '../../Logic/getProjectSelectFilter';
import { getProjectCodeWithTitle } from '../../Logic/projectCode';

export const DebouncedSelectProjectMulti = ({formitem=StyledFormItem, name='projectFilter', ...props}) => {
  return <DebouncedSelect
    dropdownAlignBottom
    name={name}
    label="Код и наименование проекта"
    mode={'tags'}
    formitem={{ formitem: formitem }}
    query={QUERY_PROJECTS}
    getQueryVariables={getProjectSelectFilter}
    placeholderSpec={{
      // svg: AnchorIcon,
      title: 'Код Наименование',
      // titleMarginLeft: '-.5rem',
      needSvgInDropdown: false,
    }}
    valueSelector={(node) => node?.id}
    queryKey="searchProject"
    dataUnpackSpec={{
      unpackForLocalCompare: (a) => getProjectCodeWithTitle(a.startDate, a.numInYear, a.title),
    }}
    {...props}
  />;
}
