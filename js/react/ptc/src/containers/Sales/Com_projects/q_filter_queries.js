import {gql} from '@apollo/client';

export const QUERY_SEARCH_BRAND = gql`
query SearchBrand {
  searchBrand {
    edges {
      node {
        id,
        title
      }
    }
  }
}
`;

export const QUERY_SEARCH_PROJECT_NAME = gql`
query searchProject($title_Icontains: String) {
  searchProject(title_Icontains: $title_Icontains, first: 3) {
    edges {
      node {
        id,
        title
      }
    }
  }
}
`;
