import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import { QUERY_ADVERTISER } from '../../../containers/Installations/Projects/queries';
import { ReactComponent as IconAdvertiser } from '../../../img/left-bar/filter/advertiser.svg';
import React from 'react';
import { StyledFormItem } from './StyledFormItem';


export const DebouncedSelectClient = ({formitem=StyledFormItem, name='clientId', ...props}) => {
  return <DebouncedSelect
    dropdownAlignBottom
    name={name}
    label="Рекламодатель"
    formitem={{ formitem: formitem }}
    query={QUERY_ADVERTISER}
    getQueryVariables={(term) => ({ title_Icontains: term })}
    placeholderSpec={{
      svg: IconAdvertiser,
      title: 'Рекламодатель',
      svgMarginTop: 0,
      needSvgInDropdown: true,
      titleMarginLeft: '-.5rem',
    }}
    valueSelector={(node) => node?.id}
    queryKey="searchPartner"
    dataUnpackSpec={{ unpackNodeKey: 'title' }}
    {...props}
  />
}
