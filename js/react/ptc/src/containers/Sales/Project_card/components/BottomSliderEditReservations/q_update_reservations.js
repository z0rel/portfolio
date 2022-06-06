import { gql } from '@apollo/client';

export const UPDATER_RESERVATIONS = gql`
  mutation CreateOrUpdateReservation(
    $dateFrom: DateTime!
    $dateTo: DateTime!
    $project: ID!
    $branding: Boolean
    $reservationType: ID
    $idsSelected: [ID]
  ) {
    createOrUpdateReservation(
      branding: $branding
      dateFrom: $dateFrom
      dateTo: $dateTo
      project: $project
      reservationType: $reservationType
      ids: $idsSelected
    ) {
      ok
    }
  }
`;
