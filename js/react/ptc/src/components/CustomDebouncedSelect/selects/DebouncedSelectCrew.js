import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import React from 'react';
import { ReactComponent as OwnerIcon } from '../../../img/input/owner.svg';
import { gql } from '@apollo/client';

export const DebouncedSelectCrew = ({ formitem = StyledFormItem, name = 'crew', ...props }) => {
  return (
    <DebouncedSelect
      className="crew"
      name="crew"
      label="Экипаж"
      dropdownAlignTop
      rules={[{ required: true, message: 'Пожалуйста, выберите экипаж.' }]}
      formitem={{ formitem: formitem }}
      query={QUERY_DESIGNS}
      getQueryVariables={(term) => {
          return { name_Icontains: term };
      }}
      placeholderSpec={{
        svg: OwnerIcon,
        title: 'Экипаж',
        svgMarginTop: 0,
        needSvgInDropdown: true,
        titleMarginLeft: '-.5rem',
      }}
      valueSelector={(node) => node?.id}
      queryKey="searchCrew"
      dataUnpackSpec={{ unpackNodeKey: 'name' }}
      dataUnpack={(data) => {
        return data?.searchCrew?.edges;
      }}
      {...props}
    />
  );
};

const QUERY_DESIGNS = gql`
  query searchCrew($name_Icontains: String) {
    searchCrew(name_Icontains: $name_Icontains) {
      edges {
        node {
          id
          name
        }
      }
    }
  }
`;
