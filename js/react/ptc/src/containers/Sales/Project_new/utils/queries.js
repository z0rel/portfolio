import { gql } from '@apollo/client';

export const PROJECT_CREATOR = gql`
  mutation($input: CreateProjectInput!) {
    createProject(input: $input) {
      project {
        id
        code
        numInYear
      }
    }
  }
`;

export const GET_MANAGERS = gql`
  query {
    searchUser {
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

export const GET_BRANDS = gql`
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

export const GET_WORK_SECTOR = gql`
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

export const GET_ADVERTISER = gql`
  query {
    searchPartner {
      edges {
        node {
          id
          title
        }
      }
    }
  }
`;
