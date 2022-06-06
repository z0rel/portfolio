import { gql } from '@apollo/client';

export const CITIES_QUERY = gql`
  query SearchCity($title_Icontains: String) {
    searchCity(title_Icontains: $title_Icontains) {
      edges {
        node {
          title
          id
        }
      }
    }
  }
`;


export const DELETE_ESTIMATE_ITEM = gql`
  mutation DeleteEstimateItem($id: ID!, $appendixId: ID) {
    deleteEstimateItem(id: $id, appendixId: $appendixId) {
      ok
    }
  }
`;


export const EDIT_ESTIMATE_ITEMS = gql`
  mutation EditEstimateItem(
    $ids: [ID],
    $packageEditing: Boolean,
    $nonRts: EstimateNonrtsArgument,
    $rtsAdditional: EstimateRtsAdditionalArgument,
    $rtsReservations: EstimateRtsReservation
    $projectId: ID,
    $isAll: Boolean,
    $isPackage: Boolean
  ) { editEstimateItem(
      ids: $ids,
      packageEditing: $packageEditing,
      nonRts: $nonRts,
      rtsAdditional: $rtsAdditional,
      rtsReservations: $rtsReservations,
      isAll: $isAll,
      projectId: $projectId,
      isPackage: $isPackage) {
      ok
    }
  }
`;
