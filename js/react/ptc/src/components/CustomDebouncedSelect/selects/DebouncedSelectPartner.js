import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import React from 'react';
import { ReactComponent as PortfolioIcon } from '../../../img/input/portfolio.svg';
import { gql } from '@apollo/client';

export const DebouncedSelectPartner = ({
  formitem = StyledFormItem,
  name = 'contragent',
  label = 'Наименование контрагента',
  required = true,
  ...props
}) => {
  return (
    <DebouncedSelect
      dropdownAlignBottom
      name={name}
      rules={{ message: 'Пожалуйста, выберите контрагента', required: required }}
      label={label}
      query={SEARCH_PARTNER}
      getQueryVariables={(term) => {
        return { title_Icontains: term };
      }}
      formitem={{ formitem: formitem }}
      placeholderSpec={{
        svg: PortfolioIcon,
        title: 'Наименование контрагента',
        svgMarginTop: '.13rem',
        titleMarginLeft: '-.5rem',
        needSvgInDropdown: true,
        svgWidthAttr: '18px',
      }}
      queryKey="searchPartner"
      valueSelector={(node) => node?.id}
      dataUnpackSpec={{ unpackNodeKey: 'title' }}
      style
      {...props}
    />
  );
};

const SEARCH_PARTNER = gql`
  query searchPrther($title_Icontains: String) {
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
