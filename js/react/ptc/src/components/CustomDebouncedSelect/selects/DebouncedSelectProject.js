import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import { QUERY_PROJECTS } from '../../../containers/Installations/Orders/queries';
import React from 'react';
import getProjectSelectFilter from '../../Logic/getProjectSelectFilter';
import { ReactComponent as AnchorIcon } from '../../../img/input/anchor.svg';
import { getProjectCodeWithTitle } from '../../Logic/projectCode';

export const DebouncedSelectProject = ({ formitem = StyledFormItem, name = 'project', ...props }) => {
  return (
    <DebouncedSelect
      dropdownAlignBottom
      name={name}
      label="Название проекта"
      formitem={{ formitem: formitem }}
      query={QUERY_PROJECTS}
      getQueryVariables={getProjectSelectFilter}
      placeholderSpec={{
        svg: AnchorIcon,
        title: 'Выбрать проект',
        svgMarginTop: '.14rem',
        titleMarginLeft: '-.5rem',
      }}
      valueSelector={(node) => node?.id}
      queryKey="searchProject"
      dataUnpackSpec={{
        unpackForLocalCompare: (a) => getProjectCodeWithTitle(a.startDate, a.numInYear, a.title),
      }}
      {...props}
    />
  );
};
