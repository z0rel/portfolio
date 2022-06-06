import { gql } from '@apollo/client';


export const QUERY_INVOICE = gql`
  query SearchSalesInvoice {
    searchSalesInvoice {
      edges {
        node {
          id
          sumWithoutNds
          wholeSum
          paymentLastDate
          customerPaymentMethod
          avr
          partner {
            title
          }
          contract {
            code
          }
          appendix {
            code

            project {
              code
              title
              brand {
                title
              }
              startDate
              client {
                title
              }
              agency {
                title
              }
            }
          }
        }
      }
    }
  }
`;
