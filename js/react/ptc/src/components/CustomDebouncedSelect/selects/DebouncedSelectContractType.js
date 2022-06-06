import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import React from 'react';
import { ReactComponent as ContractIcon } from '../../../img/input/contract.svg';
import { gql } from '@apollo/client';

export const DebouncedSelectContractType = ({
  formitem = StyledFormItem,
  name = 'contractType',
  ...props
}) => {
  return (
    <DebouncedSelect // Тип договора
      dropdownAlignBottom
      name={name}
      rules={{ message: 'Пожалуйста, выберите тип договора', required: true }}
      label="Тип договора"
      query={SEARCH_CONTRACT_TYPE}
      getQueryVariables={(term) => {
        return { name_Icontains: term };
      }}
      formitem={{ formitem: formitem }}
      placeholderSpec={{
        svg: ContractIcon,
        title: 'Тип договора',
        svgMarginTop: '.13rem',
        titleMarginLeft: '-.5rem',
        needSvgInDropdown: true,
        svgWidthAttr: '18px',
      }}
      queryKey="searchContractType"
      dataUnpackSpec={{ unpackNodeKey: 'name' }}
      style
      {...props}
    />
  );
};

const SEARCH_CONTRACT_TYPE = gql`
  query searchContractType($name_Icontains: String) {
    searchContractType(name_Icontains: $name_Icontains) {
      edges {
        node {
          id
          name
        }
      }
    }
  }
`;
