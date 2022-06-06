import { gql } from '@apollo/client';

export const QUERY_CITIES = gql`
  query SearchCities($title_Icontains: String){
    searchCity(title_Icontains: $title_Icontains) {
      edges {
        node {
          id
          title
        }
      }
    }
  }
`;

export const QUERY_DISTRICTS = gql`
  query SearchDistrict($city_Id: ID, $title_Icontains: String) {
    searchDistrict(city_Id: $city_Id, title_Icontains: $title_Icontains) {
      edges {
        node {
          id
          title
        }
      }
    }
  }
`;

export const QUERY_MARKETING_ADDRESS = gql`
  query SearchLocAdress($city_Id: ID, $district_Id: ID, $address_Icontains: String) {
    searchLocAdress(postcode_District_City_Id: $city_Id, postcode_District_Id: $district_Id, address_Icontains: $address_Icontains) {
      edges {
        node {
          address
        }
      }
    }
  }
`;

export const QUERY_FAMILIES = gql`
  query searchFamilyConstruction($title_Icontains: String) {
    searchFamilyConstruction(title_Icontains: $title_Icontains) {
      edges {
        node {
          id
          title
        }
      }
    }
  }
`;

export const QUERY_FORMAT = gql`
  query searchFormatTitles(
    $model_Underfamily_Family_Id: ID,
    $sideTitle: String,
    $title_Icontains: String,
  ) {
    searchFormatTitles(
      model_Underfamily_Family_Id: $model_Underfamily_Family_Id,
      sideTitle: $sideTitle,
      title_Icontains: $title_Icontains,
    ) {
      formatTitles {
        edges {
          node {
            id
            title
          }
        }
      }
    }
  }
`;

export const QUERY_SIDE = gql`
  query searchSideTitles(
    $format_Model_Underfamily_Family_Id: ID,
    $format_Title: String,
    $title_Icontains: String,
    $size: String,
  ) {
    searchSideTitles(
      format_Model_Underfamily_Family_Id: $format_Model_Underfamily_Family_Id,
      format_Title: $format_Title,
      title_Icontains: $title_Icontains,
      size: $size
    ) {
      sideSize {
        edges {
          node {
            id
            title
          }
        }
      }
    }
  }
`;

export const QUERY_SIZE = gql`
  query searchSideSize(
    $format_Model_Underfamily_Family_Id: ID,
    $format_Title: String,
    $title: String,
    $size_Icontains: String,
  ) {
    searchSideSize(
      format_Model_Underfamily_Family_Id: $format_Model_Underfamily_Family_Id,
      format_Title: $format_Title,
      title: $title,
      size_Icontains: $size_Icontains
    ) {
      sideSize {
        edges {
          node {
            id
            size
          }
        }
      }
    }
  }
`;

export const QUERY_SEARCH_PARTNER = gql`
  query searchPartner($title_Icontains: String) {
    searchPartner(isNonrtsOwner: true, title_Icontains: $title_Icontains) {
      edges {
        node {
          id
          title
        }
      }
    }
  }
`;

