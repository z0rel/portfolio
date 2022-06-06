import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import { ReactComponent as AnchorIcon } from '../../../img/input/anchor.svg';
import React from 'react';
import { gql } from '@apollo/client';


export const DebouncedSelectTechProblem = ({ formitem = StyledFormItem, name = 'techProblem', ...props }) => {

  return (
    <DebouncedSelect
      dropdownAlignBottom
      name={name}
      label="Техническая проблема"
      formitem={{ formitem: formitem }}
      query={QUERY_TECH_PROBLEM}
      getQueryVariables={(term) => ({ title_Icontains: term })}
      placeholderSpec={{
        title: 'Техническая проблема'
      }}
      valueSelector={(node) => node?.id}
      queryKey="searchTechProblem"
      dataPredicate={(data) => (data?.searchTechProblem?.edges.length || -1) > 0}
      dataUnpackSpec={{ unpackNodeKey: 'title' }}
      mode={'tags'}
      {...props}
    />
  );
};

export const QUERY_TECH_PROBLEM = gql`
  query searchTechProblem(
    $title_Icontains: String,
  ) {
    searchTechProblem(
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
