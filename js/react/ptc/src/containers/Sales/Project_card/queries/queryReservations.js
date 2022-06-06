import { gql } from '@apollo/client';

export const QUERY_PROJECT_CARD_RESERVATIONS = gql`
  query QueryProjectCardReservations(
    $projectId: ID,
    $limit: Int,
    $offset: String,
    $orderBy: String,
    $fastSearch: String
  ) {
    searchReservation(
      projectId: $projectId,
      first: $limit,
      after: $offset,
      orderBy: $orderBy,
      fastSearch: $fastSearch
    ) {
      pageInfo {
        startCursor,
        endCursor,
        totalCount
      },
      edges {
        node {
          id
          dateFrom
          dateTo
          creationDate
          branding
          constructionSide {
            package {
              title
            }
            advertisingSide {
              code
              title
              side {
                title
                code
                format {
                  code
                  title
                }
              }
            }
            construction {
              numInDistrict
              statusConnection
              location {
                marketingAddress {
                  address
                }
                postcode {
                  title
                  district {
                    city {
                      title
                    }
                  }
                }
              }
            }
          }
          reservationType {
            title
            id
          }
        }
      }
    }
  }
`;
