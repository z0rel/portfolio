import { gql } from '@apollo/client';

export const QUERY_SEARCH_PROJECT = gql`
  query searchProject($id: ID) {
    searchProject(id: $id) {
      edges {
        node {
          id
          title
          code
          creator {
            id
            firstName
            lastName
          }
          backOfficeManager {
            id
            firstName
            lastName
          }
          salesManager {
            id
            firstName
            lastName
          }
          comment
          brand {
            id
            title
            workingSector {
              id
              description
              title
            }
          }
          agency {
            id
            title
          }
          client {
            id
            title
          }
          agencyCommission {
            value
            percent
            toAdditional
            toMount
            toNalog
            toNonrts
            toPrint
            toRent
            agent {
              id
              title
            }
          }
        }
      }
    }
  }
`;

export const UPDATE_PROJECT = gql`
  mutation updateProject($id: ID!, $input: UpdateProjectInput!) {
    updateProject(id: $id, input: $input) {
      project {
        id
        title
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
