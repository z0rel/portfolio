import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import { ReactComponent as AnchorIcon } from '../../../img/input/anchor.svg';
import React from 'react';
import { gql } from '@apollo/client';

export const DebouncedSelectUnderfamily = ({ formitem = StyledFormItem, familyID, name = 'underfamily', svg = AnchorIcon, ...props }) => {
  return (
    <DebouncedSelect
      dropdownAlignBottom
      name={name}
      label="Подсемейство конструкции"
      formitem={{ formitem: formitem }}
      query={GET_UNDERFAMILIES}
      getQueryVariables={(term) => ({ title_Icontains: term })}
      placeholderSpec={{
        svg: svg,
        title: 'Подсемейство',
        svgMarginTop: '.06rem',
        needSvgInDropdown: true,
        titleMarginLeft: '-.5rem',
      }}
      valueSelector={(node) => node?.id}
      nodeToTitle={(node) => node?.title}
      queryKey="searchUnderFamilyConstruction"
      dataUnpackSpec={{ unpackNodeKey: 'title' }}
      additionalVars={{id :familyID}}
      {...props}
    />
  );
};

const GET_UNDERFAMILIES = gql`
  query SearchUnderFamily($id: ID, $title: String) {
    searchUnderFamilyConstruction(family_Id: $id, title_Icontains: $title) {
      edges {
        node {
          id
          title
        }
      }
    }
  }
`;
