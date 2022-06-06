import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import { ReactComponent as AnchorIcon } from '../../../img/input/anchor.svg';
import React from 'react';
import { gql } from '@apollo/client';

export const DebouncedSelectSize = ({ formitem = StyledFormItem, name = 'size',  format_Title, required,  ...props }) => {
  return (
    <DebouncedSelect
      dropdownAlignBottom
      name={name}
      label="Размер кострукции"
      formitem={{ formitem: formitem }}
      rules={{ message: 'Пожалуйста выберите размер', required: required }}
      query={QUERY_SIZE}
      getQueryVariables={(term) => ({ title_Icontains: term, format_Title: format_Title })}
      placeholderSpec={{
        svg: AnchorIcon,
        title: 'Размер',
        svgMarginTop: '.3rem',
        needSvgInDropdown: true,
        titleMarginLeft: '-.5rem',
      }}
      valueSelector={(node) => node?.id}
      nodeToTitle={(node) => node?.size}
      queryKey="searchSideSize"
      dataPredicate={(data) => (data?.searchSideSize?.sideSize?.edges.length || -1) > 0}
      dataUnpackSpec={{ unpackNodeKey: 'size' }}
      dataUnpack={(data) => data?.searchSideSize?.sideSize?.edges}
      {...props}
    />
  );
};

export const QUERY_SIZE = gql`
  query searchSideSize(
    $format_Model_Underfamily_Family_Id: ID,
    $format_Title: String,
    $title_Icontains: String,
    $size_Icontains: String,
  ) {
    searchSideSize(
      format_Model_Underfamily_Family_Id: $format_Model_Underfamily_Family_Id,
      format_Title: $format_Title,
      title_Icontains: $title_Icontains,
      size_Icontains: $size_Icontains
    ) {
      sideSize {
        edges {
          node {
            id
            size
          }
        }
      }
    }
  }
`;