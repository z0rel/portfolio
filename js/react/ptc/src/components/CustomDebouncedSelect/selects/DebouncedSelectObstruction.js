import { StyledFormItem } from './StyledFormItem';
import { DebouncedInput } from '../../SearchInput/DebouncedInput';
import { ReactComponent as AnchorIcon } from '../../../img/input/anchor.svg';
import React from 'react';
import { gql } from '@apollo/client';

export const DebouncedSelectObstruction = ({ formitem = StyledFormItem, name = 'obstruction', ...props }) => {
  return (
    <DebouncedInput
      dropdownAlignBottom
      name={name}
      label="Помеха"
      formitem={{ formitem: formitem }}
      query={QUERY_OBSTRUCTIONS}
      getQueryVariables={(term) => ({ title_Icontains: term })}
      placeholderSpec={{
        svg: AnchorIcon,
        title: 'Помеха',
        svgMarginTop: '.3rem',
        needSvgInDropdown: true,
        titleMarginLeft: '-.5rem',
      }}
      valueSelector={(node) => node?.id}
      nodeToTitle={(node) => node?.title}
      queryKey="searchObstruction"
      dataPredicate={(data) => (data?.searchObstruction?.edges.length || -1) > 0}
      dataUnpackSpec={{ unpackNodeKey: 'title' }}
      {...props}
    />
  );
};

export const QUERY_OBSTRUCTIONS = gql`
  query searchObstruction(
    $title_Icontains: String,
  ) {
    searchObstruction(
      title_Icontains: $title_Icontains,
    ) {
        edges {
          node {
            id
            title
          }
      }
    }
  }
`;
