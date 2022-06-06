import { gql } from '@apollo/client';

export const QUERY_PROJECT_CARD_APPENDICES = gql`
  query QueryProjectCardAppendices(
    $projectId: ID,
    $limit: Int,
    $offset: String,
    $orderBy: String,
    $fastSearch: String
  ) {
    searchAppendix(
      projectId: $projectId,
      first: $limit,
      after: $offset,
      orderBy: $orderBy,
      fastSearch: $fastSearch
    ) {
      edges {
        node {
          id
          code
          createdDate
          periodStartDate
          periodEndDate
          returnStatus
          salesManager {
            id
            firstName
            lastName
          }
          creator {
            id
            firstName
            lastName
          }
          signatoryOne
          signatoryTwo
          paymentDate
          signatoryPosition
        }
      }
    }
  }
`;
