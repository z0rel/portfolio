import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import React from 'react';
import { ReactComponent as IconDesign } from '../../../img/sales/projectDropdown/design.svg';
import { gql } from '@apollo/client';

export const DebouncedSelectDesign = ({ formitem = StyledFormItem, projectId = undefined, name = 'designId', ...props }) => {
  return (
    <DebouncedSelect
      className="design"
      name="designId"
      label="Дизайн"
      dropdownAlignTop
      rules={[{ required: true, message: 'Пожалуйста, выберите дизайн.' }]}
      formitem={{ formitem: formitem }}
      query={QUERY_DESIGNS}
      getQueryVariables={(term) => {
        return { title_Icontains: term, projectId: projectId };
      }}
      placeholderSpec={{
        svg: IconDesign,
        title: 'Дизайн',
        svgMarginTop: 0,
        needSvgInDropdown: true,
        titleMarginLeft: '-.5rem',
      }}
      valueSelector={(node) => node?.id}
      queryKey="searchDesign"
      dataUnpackSpec={{ unpackNodeKey: 'title' }}
      dataUnpack={(data) => {
        return data?.searchDesign?.edges;
      }}
      {...props}
    />
  );
};

const QUERY_DESIGNS = gql`
  query SearchDesign($projectId: ID, $title_Icontains: String) {
    searchDesign(advertPromoCompany_ProjectId: $projectId, title_Icontains: $title_Icontains) {
      edges {
        node {
          id
          title
        }
      }
    }
  }
`;
