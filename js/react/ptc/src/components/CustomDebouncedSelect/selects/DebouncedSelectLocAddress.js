import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import { ReactComponent as AnchorIcon } from '../../../img/input/anchor.svg';
import React from 'react';
import { gql } from '@apollo/client';


export const DebouncedSelectLocAddress = ({ formitem = StyledFormItem, name = 'adress_m', label = 'Адрес', cityId, districtId, initialTerm, svg = AnchorIcon, ...props }) => {
  return (
    <DebouncedSelect
      dropdownAlignBottom
      name={name}
      label={label}
      formitem={{ formitem: formitem }}
      query={QUERY_MARKETING_ADDRESS}
      getQueryVariables={(term) => {
        return { city_Id: cityId, district_Id: districtId, address_Icontains: term || initialTerm };
      }}
      placeholderSpec={{
        svg: svg,
        title: label,
        svgMarginTop: '.14rem',
        needSvgInDropdown: true,
        titleMarginLeft: '-.5rem',
      }}
      valueSelector={(node) => node?.id}
      queryKey="searchLocAdress"
      dataPredicate={(data) => (data?.searchLocAdress?.edges.length || -1) > 0}
      dataUnpackSpec={{ unpackNodeKey: 'address' }}
      dataUnpack={(data) => {
        return data?.searchLocAdress?.edges.filter(
          (node) => node.node.address !== '' && node.node.address !== null,
        );
      }}
      {...props}
    />
  );
};

const QUERY_MARKETING_ADDRESS = gql`
  query SearchLocAdress($city_Id: ID, $district_Id: ID, $address_Icontains: String) {
    searchLocAdress(postcode_District_City_Id: $city_Id, postcode_District_Id: $district_Id, address_Icontains: $address_Icontains) {
      edges {
        node {
          id
          address
        }
      }
    }
  }
`;
