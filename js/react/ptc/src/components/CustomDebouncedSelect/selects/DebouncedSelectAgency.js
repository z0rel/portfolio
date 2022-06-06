import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import { QUERY_AGENCY } from '../../../containers/Installations/Projects/queries';
import React from 'react';
import { ReactComponent as IconAdvAgency } from '../../../img/left-bar/filter/advAgency.svg';

export const DebouncedSelectAgency = ({formitem=StyledFormItem, name='agencyId', ...props}) => {
  return <DebouncedSelect
    dropdownAlignBottom
    name={name}
    label="Рекламное агентство"
    formitem={{ formitem: formitem }}
    query={QUERY_AGENCY}
    getQueryVariables={(term) => ({ title_Icontains: term })}
    placeholderSpec={{
      svg: IconAdvAgency,
      title: 'Рекламное агентство',
      svgMarginTop: 0,
      needSvgInDropdown: true,
      titleMarginLeft: '-.5rem',
    }}
    valueSelector={(node) => node?.id}
    queryKey="searchPartner"
    dataUnpackSpec={{ unpackNodeKey: 'title' }}
    {...props}
  />;
}
