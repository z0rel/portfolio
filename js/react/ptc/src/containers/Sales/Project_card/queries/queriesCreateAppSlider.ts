import { gql } from '@apollo/client';

const QUERY_MANAGERS = gql`
query SearchUsers($firstName_Icontains: String, $lastName_Icontains: String) {
  searchUser(firstName_Icontains: $firstName_Icontains, lastName_Icontains: $lastName_Icontains) {
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
const CREATE_APPENDIX = gql`
mutation CreateAppendix(
  $reservations: [ID],
  $signatoryOne: String,
  $signatoryTwo: String,
  $project: ID,
  $creator: ID,
  $salesManager: ID
) {
  createAppendix(input: {
    reservations: $reservations,
    signatoryOne: $signatoryOne,
    signatoryTwo: $signatoryTwo,
    project: $project,
    creator: $creator,
    salesManager: $salesManager,
  }) {
    appendix {
      id
      reservations {
        edges {
          node {
            id
          }
        }
      }
    }
  }
}
`;

const UPDATE_APPENDIX = gql`
mutation UpdateAppendix(
  $id: ID!,
  $reservations: [ID],
  $signatoryOne: String,
  $signatoryTwo: String,
  $project: ID,
  $creator: ID,
  $salesManager: ID
) {
  updateAppendix(
    id: $id
    input: {
      reservations: $reservations,
      signatoryOne: $signatoryOne,
      signatoryTwo: $signatoryTwo,
      project: $project,
      creator: $creator,
      salesManager: $salesManager,
    }) {
    appendix {
      id
      reservations {
        edges {
          node {
            id
          }
        }
      }
    }
  }
}
`;

export {
  QUERY_MANAGERS,
  SEARCH_CREATORS,
  CREATE_APPENDIX,
  UPDATE_APPENDIX,
}
