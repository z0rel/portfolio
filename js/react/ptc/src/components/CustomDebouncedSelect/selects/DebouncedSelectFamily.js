import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import { ReactComponent as AnchorIcon } from '../../../img/input/anchor.svg';
import React from 'react';
import { gql } from '@apollo/client';

export const DebouncedSelectFamily = ({ formitem = StyledFormItem, name = 'family', svg = AnchorIcon, ...props }) => {
  return (
    <DebouncedSelect
      dropdownAlignBottom
      name={name}
      label="Семейство конструкции"
      formitem={{ formitem: formitem }}
      query={QUERY_FAMILIES}
      getQueryVariables={(term) => ({ title_Icontains: term })}
      placeholderSpec={{
        svg: svg,
        title: 'Семейство',
        svgMarginTop: '.06rem',
        needSvgInDropdown: true,
        titleMarginLeft: '-.5rem',
      }}
      valueSelector={(node) => node?.id}
      nodeToTitle={(node) => node?.title}
      queryKey="searchFamilyConstruction"
      dataUnpackSpec={{ unpackNodeKey: 'title' }}
      {...props}
    />
  );
};

const QUERY_FAMILIES = gql`
  query searchFamilyConstruction($title_Icontains: String) {
    searchFamilyConstruction(title_Icontains: $title_Icontains) {
      edges {
        node {
          id
          title
        }
      }
    }
  }
`;
