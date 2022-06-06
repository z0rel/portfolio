import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import React from 'react';
import { ReactComponent as ProjectIcon } from '../../../img/sales/projectNameInput.svg';
import { gql } from '@apollo/client';

export const DebouncedSelectAppendix = ({ formitem = StyledFormItem, name = 'cityId', projectId, ...props }) => {
  return (
    <DebouncedSelect // Приложение +
      dropdownAlignTop
      disabled={false}
      name="appendix"
      rules={{ message: 'Пожалуйста выберите приложение', required: true }}
      label="Приложение"
      formitem={{ antd: false }}
      query={QUERY_APPENDIX}
      getQueryVariables={(term) => ({ title_Icontains: term, projectId: projectId })}
      placeholderSpec={{
        svg: ProjectIcon,
        title: 'Название приложения',
      }}
      valueSelector={(node) => node.id}
      nodeToTitle={(node) => (node.code ? node.code : 'Нет названия')}
      queryKey="searchAppendix"
      dataUnpackSpec={{ unpackNodeKey: 'code' }}
      {...props}
    />
  );
};

const QUERY_APPENDIX = gql`
  query SearchAppendix($projectId: ID, $title_Icontains: String) {
    searchAppendix(project_Id: $projectId, code_Icontains: $title_Icontains) {
      edges {
        node {
          id
          code
        }
      }
    }
  }
`;
