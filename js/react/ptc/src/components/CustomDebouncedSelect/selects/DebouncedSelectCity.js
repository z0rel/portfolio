import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import { QUERY_CITIES } from '../../../containers/Installations/Projects/queries';
import React from 'react';
import { ReactComponent as CityIcon } from '../../../img/input/city.svg';

export const DebouncedSelectCity = ({ formitem = StyledFormItem, name = 'cityId', ...props }) => {
  return (
    <DebouncedSelect
      dropdownAlignBottom
      name={name}
      label="Город"
      formitem={{ formitem: formitem }}
      query={QUERY_CITIES}
      getQueryVariables={(term) => ({ title_Icontains: term })}
      placeholderSpec={{
        svg: CityIcon,
        title: 'Выберите город',
        svgMarginTop: 0,
        needSvgInDropdown: true,
        titleMarginLeft: '-.5rem',
      }}
      valueSelector={(node) => node?.id}
      queryKey="searchCity"
      dataUnpackSpec={{ unpackNodeKey: 'title' }}
      {...props}
    />
  );
};
