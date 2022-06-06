import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import React from 'react';
import { ReactComponent as ProjectNameInput } from '../../../img/sales/projectNameInput.svg';
import { gql } from '@apollo/client';

export const DebouncedSelectReservationStatus = ({
                                                   isLongPlaceholder = undefined,
                                                   formitem = StyledFormItem,
                                                   name = 'status',
                                                   ...props
                                                 }) => {
  return (
    <DebouncedSelect // Статус бронирования
      className="reservation-type"
      name={name}
      rules={[{ required: true, message: 'Пожалуйста, выберите статус бронирования.' }]}
      label="Статус бронирования"
      dropdownAlignTop
      disabled={false}
      formitem={formitem}
      query={QUERY_RESERVATIONS_STATUS}
      getQueryVariables={(term) => {
        return { title_Icontains: term };
      }}
      placeholderSpec={{
        svg: ProjectNameInput,
        title: isLongPlaceholder || isLongPlaceholder === undefined ? 'Статус бронирования' : 'Статус',
      }}
      valueSelector={(node) => node.id}
      queryKey="searchReservationType"
      dataUnpack={(query) => {
        let nodes = query?.searchReservationType?.edges.filter(
          (node) =>
            node?.node.title !== 'Недоступно' && node?.node.title !== 'Свободно' && node?.node.title !== 'Отменено',
        );
        nodes.sort((a, b) => a.node.level - b.node.level || a.node.title.localeCompare(b.node.title));
        return nodes;
      }}
      {...props}
    />
  );
};

const QUERY_RESERVATIONS_STATUS = gql`
  query {
    searchReservationType {
      edges {
        node {
          id
          title
        }
      }
    }
  }
`;
