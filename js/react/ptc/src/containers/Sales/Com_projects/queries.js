import { gql } from '@apollo/client';

export const QUERY_ALL_PROJECTS = gql`
  query allProjectsQuery(
    $projectFilterspecOr: [SearchCommercialProjectSpec],
    $brandId: ID,
    $clientId: ID,
    $agencyId: ID,
    $sectorId: ID,
    $backOfficeManagerId: ID,
    $salesManagerId: ID,
    $createdAtGte: DateTime,
    $createdAtLte: DateTime,
    $orderBy: [MountingCommercialProjectsOrderBy]
    $limit: Int,
    $offset: Int,
    $fastSearch: String
  ) {
    searchProjectsOptim(
      projectFilterspecOr: $projectFilterspecOr,
      brandId: $brandId,
      clientId: $clientId,
      agencyId: $agencyId,
      workingSectorId: $sectorId,
      backOfficeManagerId: $backOfficeManagerId,
      salesManagerId: $salesManagerId,
      createdAt_Gte: $createdAtGte,
      createdAt_Lte: $createdAtLte,
      orderBy: $orderBy,
      limit: $limit,
      offset: $offset
      fastSearch: $fastSearch
    ) {
      pageInfo {
        totalCount
      }
      content
    }
  }
`;

export const QUERY_WORKING_SECTOR = gql`
  query SearchWorkingSector($description_Icontains: String) {
    searchWorkingSector(description_Icontains: $description_Icontains) {
      edges {
        node {
          id
          description
        }
      }
    }
  }
`;
