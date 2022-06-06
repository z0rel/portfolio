
const microdata = {
    OFFER_CATALOG: {itemProp: "hasOfferCatalog", itemScope: true, itemType: "http://schema.org/OfferCatalog"},
    ITEM_LIST_OFFER: {itemProp: "itemListElement", itemScope: true, itemType: "http://schema.org/Offer"},
    ITEM_OFFERED_SERVICE: { itemProp: "itemOffered", itemScope: true, itemType: "http://schema.org/Service"},
    MAKES_OFFER: { itemProp: "makesOffer", itemScope: true, itemType: "http://schema.org/Offer" }
}

export default microdata;