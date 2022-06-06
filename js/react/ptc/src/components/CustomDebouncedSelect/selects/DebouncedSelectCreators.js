import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import React from 'react';
import { ReactComponent as OwnerIcon } from '../../../img/input/owner.svg';
import { gql } from '@apollo/client';
import { getManagerName, managerSelectFilter } from '../../Logic/getManager';

export const DebouncedSelectCreators = ({
                                          formitem = StyledFormItem,
                                          name = 'creator',
                                          customFormItem = undefined,
                                          placeholderSpec = undefined,
                                          label = 'Создатель',
                                          message = 'Пожалуйста, выберите создателя',
                                          ...props
                                        }) => {
  if (props.dropdownAlignBottom === undefined && props.dropdownAlignTop === undefined) {
    props.dropdownAlignBottom = true;
  }
  return (
    <DebouncedSelect // Создатель
      name={name}
      rules={{ message: message, required: true }}
      label={label}
      query={SEARCH_CREATORS}
      getQueryVariables={managerSelectFilter}
      // getQueryVariables={(term) => {
      //   return { name_Icontains: term };
      // }}
      formitem={customFormItem ? customFormItem : { formitem: formitem }}
      placeholderSpec={placeholderSpec ? placeholderSpec : {
        svg: OwnerIcon,
        title: 'Создатель',
        svgMarginTop: '.13rem',
        titleMarginLeft: '-.5rem',
        needSvgInDropdown: true,
        svgWidthAttr: '18px',
      }}
      queryKey="searchUser"
      dataUnpackSpec={{ unpackForLocalCompare: getManagerName }}
      valueSelector={(node) => String(node?.id)}
      // dataUnpackSpec={{ unpackNodeFun: (node) => `${node.firstName || ''} ${node.lastName || ''}` }}
      style
      {...props}
    />
  );
};

const SEARCH_CREATORS = gql`
  query searchUser($name_Icontains: String) {
    searchUser(name_Icontains: $name_Icontains) {
      edges {
        node {
          id
          firstName
          lastName
        }
      }
    }
  }
`;
