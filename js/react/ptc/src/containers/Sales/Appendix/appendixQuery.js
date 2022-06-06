import { gql } from '@apollo/client';


export const QUERY_APPENDIX_PROTO = gql`
  query SearchAddressProgramProto($appendixId: ID) {
    searchSalesAddressProgramProto(appendix_Id: $appendixId) {
      content
    }
  }
`;


export const GENERATE_APPENDIX_DOCX = gql`
  mutation SalesGenerateAppendixDocx($appendixId: ID) {
    generateAppendixDocx(appendixId: $appendixId) {
      content
    }
  }
`;
