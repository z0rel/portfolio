import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import { ReactComponent as AnchorIcon } from '../../../img/input/anchor.svg';
import React from 'react';
import { gql } from '@apollo/client';

export const DebouncedSelectSide = ({ formitem = StyledFormItem, name = 'side', format_Title, format_Model_Underfamily_Family_Id, required, ...props }) => {
  return (
    <DebouncedSelect
      dropdownAlignBottom
      name={name}
      label="Сторона конструкции"
      rules={{ message: 'Пожалуйста выберите сторону', required: required }}
      formitem={{ formitem: formitem }}
      query={QUERY_SIDE}
      getQueryVariables={(term) => ({format_Model_Underfamily_Family_Id, title_Icontains: term, format_Title: format_Title })}
      placeholderSpec={{
        svg: AnchorIcon,
        title: 'Стророна',
        svgMarginTop: '.3rem',
        needSvgInDropdown: true,
        titleMarginLeft: '-.5rem',
      }}
      valueSelector={(node) => node?.id}
      nodeToTitle={(node) => node?.title}
      queryKey="searchSideTitles"
      dataPredicate={(data) => (data?.searchSideTitles?.sideSize?.edges.length || -1) > 0}
      dataUnpackSpec={{ unpackNodeKey: 'title' }}
      dataUnpack={(data) => data?.searchSideTitles?.sideSize?.edges}
      {...props}
    />
  );
};

export const QUERY_SIDE = gql`
  query searchSideTitles(
    $format_Model_Underfamily_Family_Id: ID,
    $format_Title: String,
    $title_Icontains: String,
    $size: String,
  ) {
    searchSideTitles(
      format_Model_Underfamily_Family_Id: $format_Model_Underfamily_Family_Id,
      format_Title: $format_Title,
      title_Icontains: $title_Icontains,
      size: $size
    ) {
      sideSize {
        edges {
          node {
            id
            title
            size
          }
        }
      }
    }
  }
`;
