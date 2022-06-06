import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import { ReactComponent as AnchorIcon } from '../../../img/input/anchor.svg';
import React from 'react';
import { gql } from '@apollo/client';

export const DebouncedSelectModel = ({ formitem = StyledFormItem, underFamilyID, name = 'model', svg = AnchorIcon, ...props }) => {
  return (
    <DebouncedSelect
      dropdownAlignBottom
      name={name}
      label="Модель"
      formitem={{ formitem: formitem }}
      query={GET_MODELS}
      getQueryVariables={(term) => ({ title_Icontains: term })}
      placeholderSpec={{
        svg: svg,
        title: 'Модель',
        svgMarginTop: '.06rem',
        needSvgInDropdown: true,
        titleMarginLeft: '-.5rem',
      }}
      valueSelector={(node) => node?.id}
      nodeToTitle={(node) => node?.title}
      queryKey="searchModelConstruction"
      dataUnpackSpec={{ unpackNodeKey: 'title' }}
      additionalVars={{id :underFamilyID}}
      {...props}
    />
  );
};

const GET_MODELS = gql`
  query SearchModelConstruction($id: ID, $title: String) {
    searchModelConstruction(underfamily_Id: $id, title_Icontains: $title) {
      edges {
        node {
          id
          title
        }
      }
    }
  }
`;
