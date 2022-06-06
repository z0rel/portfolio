import { DebouncedSelect, IsMounted } from '../../../../components/SearchSelect/DebouncedSelect';
import { CITIES_QUERY } from '../q_mutations';
import { ReactComponent as WorldIcon } from '../../../../img/header-bar/world.svg';
import React from 'react';


interface CityFormItemProps {
  isReservationNonRts: boolean;
  componentIsMounted?: IsMounted;
}


export const CityFormItem = ({isReservationNonRts, componentIsMounted, ...props}: CityFormItemProps): React.ReactElement => {
  return <DebouncedSelect // Город +
    {...props}
    dropdownAlignTop
    disabled={isReservationNonRts}
    name="city"
    rules={{message: 'Пожалуйста выберите город.', required: true}}
    label="Город"
    formitem={{antd: false}}
    query={CITIES_QUERY}
    getQueryVariables={(term) => {
      return { title_Icontains: term };
    }}
    placeholderSpec={{
      svg: WorldIcon,
      title: 'Выбрать город',
      svgMarginTop: '.15rem'
    }}
    componentIsMounted={componentIsMounted}
    queryKey="searchCity"
  />;
};
