import { gql } from '@apollo/client';

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
const SEARCH_BRANDS = gql`
  query {
    searchBrand {
      edges {
        node {
          id
          title
          workingSector {
            title
            id
          }
        }
      }
    }
  }
`;
const SEARCH_WORK_SECTOR = gql`
  query {
    searchWorkingSector {
      edges {
        node {
          id
          title
        }
      }
    }
  }
`;
const SEARCH_ADVERTISER = gql`
  query searchPartner($title_Icontains: String) {
    searchPartner(title_Icontains: $title_Icontains) {
      edges {
        node {
          id
          title
        }
      }
    }
  }
`;
export {
  SEARCH_CREATORS,
  SEARCH_BRANDS,
  SEARCH_WORK_SECTOR,
  SEARCH_ADVERTISER,
}
