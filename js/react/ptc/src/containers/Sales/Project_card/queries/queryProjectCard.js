import { gql } from '@apollo/client';

export const QUERY_PROJECT_CARD = gql`
  query QueryProjectCard($id: ID!) {
    searchProject(id: $id) {
      pageInfo {
        startCursor,
        endCursor,
        totalCount
      },
      edges {
        node {
          id
          title
          code
          createdAt
          title
          creator {
            id
            firstName
            lastName
            lastLogin
          }
          client {
            title
            id
          }
          agency {
            title
          }
          comment
          brand {
            title
            workingSector {
              title
              description
            }
          }
          salesManager {
            firstName
            lastName
          }
          backOfficeManager {
            firstName
            lastName
          }
          agencyCommission {
            value
            percent
          }
        }
      }
    }
  }
`;
