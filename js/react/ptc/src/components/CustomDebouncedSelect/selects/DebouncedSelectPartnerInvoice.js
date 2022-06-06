import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import React from 'react';
import { ReactComponent as IconAdvertiser } from '../../../img/left-bar/filter/advertiser.svg';
import { gql } from '@apollo/client';

export const DebouncedSelectPartnerInvoice = ({ formitem = StyledFormItem, name = 'partnerId', ...props }) => {
  return (
    <DebouncedSelect
      dropdownAlignBottom
      required
      name={name}
      label="На кого выставляется счет"
      rules={[{ required: true, message: 'Пожалуйста, выберите на кого выставляется счет.' }]}
      formitem={{ formitem: formitem }}
      query={QUERY_PARTNER}
      getQueryVariables={(term) => ({ title_Icontains: term })}
      placeholderSpec={{
        svg: IconAdvertiser,
        title: 'ТОО Рекламное агенство',
        svgMarginTop: 0,
        needSvgInDropdown: true,
        titleMarginLeft: '-.5rem',
      }}
      valueSelector={(node) => node?.id}
      queryKey="searchPartner"
      dataUnpackSpec={{ unpackNodeKey: 'title' }}
      {...props}
    />
  );
};


const QUERY_PARTNER = gql`
  query SearchPartners($title_Icontains: String) {
    searchPartner(title_Icontains: $title_Icontains) {
      edges {
        node {
          id
          title
        }
      }
    }
  }
`;
