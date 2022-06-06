import { gql } from '@apollo/client';


export const QUERY_SALES_ESTIMATE_PROTO = gql`
  query SearchSalesEstimateProto($projectId: ID, $appendixId: ID) {
    searchSalesEstimateProto(project_Id: $projectId, appendix_Id: $appendixId) {
      content
    }
  }
`;


/*
export const SEARCH_SALES_ESTIMATE_ITOGS = gql`
  query SearchSalesEstimateItogs($projectId: ID, $appendixId: ID) {
    searchSalesEstimateItogs(project_Id: $projectId, appendix_Id: $appendixId) {
      edges {
        node {
          rentByPrice
          rentByPriceDiscounted
          rentByPriceDiscountPercent
          rentToClent
          rentToClentDiscounted
          rentToClentDiscountPercent
          staticMounting
          staticPrinting
          staticAdditional
          nalogBeforeDiscount
          nalogAfterDiscount
          nalogDiscountPercent
          agencyCommissionValue
          agencyCommissionPercent
          nonrtsMargin
          nonrtsSale
          additionalRtsBeforeDiscount
          additionalRtsAfterDiscount
          summaryEstimateValue
          summaryEstimateValueWithoutAgencyCommission
          additionalNonrtsByTitle {
            edges {
              node {
                name
                sale
                pay
                margin
                agencyCommissionValue
              }
            }
          }
          additionalRtsByTitle {
            edges {
              node {
                name
                summaBeforeDiscount
                discountValue
                summaAfterDiscount
                discountPercent
                agencyCommissionValue
              }
            }
          }
          additionalRts {
            edges {
              node {
                id
                title
                city {
                  id
                  title
                }
                startPeriod
                endPeriod
                count
                price
                discountPercent
                category
                summaAfterDiscount
                priceAfterDiscount
                agencyCommissionPercent
                agencyCommissionValue
                valueWithoutAgencyCommission

                summaBeforeDiscount
                discountValue
                agencyCommission {
                  id
                  percent
                  value
                  toAdditional
                  toMount
                  toNalog
                  toNonrts
                  toPrint
                  toRent
                }
              }
            }
          }
          additionalNonrts {
            edges {
              node {
                id
                name
                startPeriod
                endPeriod
                count
                incomingRent
                incomingTax
                incomingPrinting
                incomingInstallation
                incomingAdditional
                incomingManufacturing
                saleRent
                saleTax
                salePrinting
                saleManufacturing
                saleInstallation
                saleAdditional
                agencyCommissionCalculated
                sale
                pay
                margin
                city {
                  id
                  title
                }
                agencyCommission {
                  id
                  percent
                  value
                  toAdditional
                  toMount
                  toNalog
                  toNonrts
                  toPrint
                  toRent
                }
              }
            }
          }
          reservationsNonrts {
            edges {
              node {
                id
                dateFrom
                dateTo
                constructionSide {
                  id
                  construction {
                    numInDistrict
                    location {
                      postcode {
                        title
                        district {
                          city {
                            id
                            title
                          }
                        }
                      }
                    }
                  }
                  advertisingSide {
                    code
                    side {
                      code
                      format {
                        code
                      }
                    }
                  }
                }
                reservationType {
                  id
                  title
                }
                nonrtsPart {
                  id
                  count
                  startPeriod
                  endPeriod
                  incomingRent
                  incomingTax
                  incomingPrinting
                  incomingManufacturing
                  incomingInstallation
                  incomingAdditional
                  incomingPrinting
                  incomingManufacturing
                  incomingInstallation
                  incomingAdditional
                  saleRent
                  saleTax
                  saleRent
                  saleTax
                  salePrinting
                  saleManufacturing
                  saleInstallation
                  saleAdditional
                }
                sale
                pay
                margin
                agencyCommissionValue
                addressTitle
                cityTitle
                formatTitle
                branding
                agencyCommission {
                  id
                  percent
                  value
                  toAdditional
                  toMount
                  toNalog
                  toNonrts
                  toPrint
                  toRent
                }
              }
            }
          }
          additionalRtsByTitle {
            edges {
              node {
                name
                summaBeforeDiscount
                discountValue
                summaAfterDiscount
                discountPercent
                agencyCommissionValue
              }
            }
          }
          additionalStaticItogs {
            edges {
              node {
                name
                price
              }
            }
          }
          reservations {
            edges {
              node {
                id
                dateFrom
                dateTo
                branding
                rentByPriceSetted
                mountingSetted
                printingSetted
                additionalSetted
                nalogSetted

                discountPricePercentSetted
                rentByPriceAfterDiscountSetted
                rentToClientSetted
                discountToClientPercentSetted
                rentToClientAfterDiscountSetted

                discountNalogPercentSetted
                nalogAfterDiscountSetted

                reservationType {
                  title
                }
                addressTitle
                formatTitle
                cityTitle
                rentByPriceCalculated
                discountPricePercentCalculated
                discountPricePercentSelected
                valueAfterDiscountPriceCalculated
                valueRentToClientSelected
                discountClientPercentSelected
                valueRentToClientSelected
                discountClientPercentSelected
                valueRentToClientSelected
                discountClientPercentSelected
                agencyCommissionPercentSelected
                agencyCommissionValueSelected
                valueRentToClientAfterDiscountSelected
                agencyCommissionRent
                additionalStaticPrinting
                additionalStaticPrintingAk
                additionalStaticPrintingAkPercent
                additionalStaticMounting
                additionalStaticMountingAk
                additionalStaticMountingAkPercent
                additionalStaticAdditional
                additionalStaticAdditionalAk
                additionalStaticAdditionalAkPerent
                additionalStaticNalog
                additionalStaticNalogDiscountPercentSelected
                additionalStaticNalogDiscountCalculated
                additionalStaticNalogValueAfterDiscount
                additionalStaticNalogAk
                additionalStaticNalogAkPercent
                itogSummary
                itogAgencyCommission
                itogSummaryWithoutAgencyCommission
                agencyCommission {
                  id
                  percent
                  value
                  toAdditional
                  toMount
                  toNalog
                  toNonrts
                  toPrint
                  toRent
                }
                constructionSide {
                  advertisingSide {
                    code
                    side {
                      title
                      code
                      format {
                        title
                        code
                      }
                      code
                    }
                  }
                  construction {
                    numInDistrict
                    location {
                      marketingAddress {
                        address
                      }
                      postcode {
                        title
                        district {
                          title
                          city {
                            title
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
`
*/
