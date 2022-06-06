import { gql } from '@apollo/client';


export const QUERY_PROJECTS = gql`
  query searchProjects($code_Icontains: String, $title_Icontains: String, $projectId: ID) {
    searchProject(code_Icontains: $code_Icontains, title_Icontains: $title_Icontains, id: $projectId) {
      edges {
        node {
          title
          startDate
          createdAt
          numInYear
          code
          id
          agencyCommission {
            percent
            value
          }
        }
      }
    }
  }
`;


export const QUERY_RESERVATION_TYPE = gql`
  query SearchReservationType($title_Icontains: String) {
    searchReservationType(title_Icontains: $title_Icontains) {
      edges {
        node {
          id
          title
          level
        }
      }
    }
  }
`;


export const CREATE_RESERVATION = gql`
  mutation CreateOrUpdateReservation(
    $dateFrom: DateTime!,
    $dateTo: DateTime!,
    $project: ID!,
    $branding: Boolean,
    $reservationType: ID,
    $sidesIds: [ID]
  ) {
    createOrUpdateReservation(
      branding: $branding,
      dateFrom: $dateFrom,
      dateTo: $dateTo,
      project: $project,
      reservationType: $reservationType,
      sidesIds: $sidesIds
    ) {
      ok
      badReservations
      badSides
      createdReservations
      updatedReservations
      changedProject
      changedAppendix
    }
  }
`;


export const SEARCH_CONSTRUCTION_SIDE_WITH_RESERVATION = gql`
  query SearchConstructionSideWithReservation(
    $dateFrom: DateTime
    $dateTo: DateTime
    $family: ID
    $format: String
    $side: String
    $size: String
    $statusConnection: Boolean
    $city: ID
    $district: ID
    $reservationType: String
    $offset: Int
    $limit: Int
    $owner: ID
    $marketingAddress: ID,
    $postcode_Icontains: String,
    $numInDistrict: Int,
    $codeFormat_Icontains: String,
    $codeSide: String,
    $codeAdvSide: String,
    $orderBy: [AdvertisingSidesOrderBy]
  ) {
    searchAdvertisingSidesOptim(
      limit: $limit
      offset: $offset
      reservation_DateFrom_Gte: $dateFrom
      reservation_DateTo_Lte: $dateTo
      advertisingSide_Side_Format_Model_Underfamily_Family_Id: $family
      advertisingSide_Side_Format_Title: $format
      advertisingSide_Side_Title: $side
      advertisingSide_Side_Size: $size
      construction_StatusConnection: $statusConnection
      construction_Location_Postcode_District_City_Id: $city
      construction_Location_Postcode_District_Id: $district
      reservationType_Title_Iregex: $reservationType
      construction_NonrtsOwner_Id: $owner
      construction_Location_MarketingAddress_Id: $marketingAddress,
      construction_Location_Postcode_Title_Icontains: $postcode_Icontains,
      construction_NumInDistrict: $numInDistrict,
      advertisingSide_Code: $codeAdvSide,
      advertisingSide_Side_Code: $codeSide,
      advertisingSide_Side_Format_Code_Icontains: $codeFormat_Icontains,
      orderBy: $orderBy
    ) {
      content
      pageInfo {
        totalCount
        offset
        limit
      }
    }
  }
`;
