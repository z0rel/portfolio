import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import { ReactComponent as AnchorIcon } from '../../../img/input/anchor.svg';
import React from 'react';
import { gql } from '@apollo/client';

export const DebouncedSelectPurposeSide = ({ formitem = StyledFormItem, name = 'purposeSide', required, ...props }) => {
  return (
    <DebouncedSelect
      dropdownAlignBottom
      name={name}
      label="Назначение стороны"
      formitem={{ formitem: formitem }}
      rules={{ message: 'Пожалуйста выберите назначение', required: required }}
      query={QUERY_PURPOSE_SIDE}
      getQueryVariables={(term) => ({ title_Icontains: term })}
      placeholderSpec={{
        svg: AnchorIcon,
        title: 'Назначение стороны',
        svgMarginTop: '.3rem',
        needSvgInDropdown: true,
        titleMarginLeft: '-.5rem',
      }}
      valueSelector={(node) => node?.id}
      nodeToTitle={(node) => node?.title}
      queryKey="searchPurposeSide"
      dataPredicate={(data) => (data?.searchPurposeSide?.edges.length || -1) > 0}
      dataUnpackSpec={{ unpackNodeKey: 'title' }}
      dataUnpack={(data) => data?.searchPurposeSide?.edges}
      {...props}
    />
  );
};

export const QUERY_PURPOSE_SIDE = gql`
query SearchPurposeSide($id: ID, $title_Icontains: String) {
  searchPurposeSide(id: $id, title_Icontains: $title_Icontains) {
    edges {
      node {
        id
        title
      }
    }
  }
}
`;