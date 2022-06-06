import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import React from 'react';
import { QUERY_WORKING_SECTOR } from '../../../containers/Sales/Com_projects/queries';
import { ReactComponent as IconSection } from '../../../img/left-bar/filter/section.svg';

export const DebouncedSelectWorkingSector = ({formitem=StyledFormItem, name='sectorId', ...props}) => {
  return <DebouncedSelect
    dropdownAlignBottom
    name={name}
    label="Сектор деятельности"
    formitem={{ formitem: formitem }}
    query={QUERY_WORKING_SECTOR}
    getQueryVariables={(term) => ({ description_Icontains: term })}
    placeholderSpec={{
      svg: IconSection,
      title: 'Сектор деятельности',
      needSvgInDropdown: true,
      titleMarginLeft: '-.5rem',
      svgWidthAttr: '15px'
    }}
    valueSelector={(node) => node?.id}
    queryKey="searchWorkingSector"
    dataUnpackSpec={{ unpackNodeKey: 'description' }}
    {...props}
  />;
}
