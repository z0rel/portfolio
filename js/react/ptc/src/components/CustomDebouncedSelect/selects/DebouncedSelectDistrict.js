import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import React from 'react';
import { QUERY_DISTRICTS } from '../../../containers/Installations/Orders/queries';
import { ReactComponent as DistrictIcon } from '../../../img/input/district.svg';

export const DebouncedSelectDistrict = ({ formitem = StyledFormItem, name = 'district', cityId, ...props }) => {
  return (
    <DebouncedSelect
      dropdownAlignBottom
      name={name}
      label="Район"
      formitem={{ formitem: formitem }}
      query={QUERY_DISTRICTS}
      getQueryVariables={(term) => ({ city_Id: cityId, title_Icontains: term })}
      placeholderSpec={{
        svg: DistrictIcon,
        title: 'Выбрать район',
        svgMarginTop: '.14rem',
        needSvgInDropdown: true,
        titleMarginLeft: '-.5rem',
      }}
      valueSelector={(node) => node?.title}
      queryKey="searchDistrict"
      dataUnpack={(data) =>
        data?.searchDistrict?.edges.filter((node) => node.node.title !== '' && node.node.title !== null)
      }
      {...props}
    />
  );
};

export const DebouncedSelectDistrictId = ({ formitem = StyledFormItem, name = 'district', cityId, ...props }) => {
  return (
    <DebouncedSelect
      dropdownAlignBottom
      name={name}
      label="Район"
      formitem={{ formitem: formitem }}
      query={QUERY_DISTRICTS}
      getQueryVariables={(term) => ({ city_Id: cityId, title_Icontains: term })}
      placeholderSpec={{
        svg: DistrictIcon,
        title: 'Выбрать район',
        svgMarginTop: '.14rem',
        needSvgInDropdown: true,
        titleMarginLeft: '-.5rem',
      }}
      valueSelector={(node) => node?.id}
      queryKey="searchDistrict"
      dataUnpack={(data) =>
        data?.searchDistrict?.edges.filter((node) => node.node.title !== '' && node.node.title !== null)
      }
      {...props}
    />
  );
};
