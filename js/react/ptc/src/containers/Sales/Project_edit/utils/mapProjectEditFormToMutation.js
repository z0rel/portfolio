export const mapProjectEditFormToMutation = (form) => {
  let values = form.getFieldsValue(true);
  return {
    ...form.getFieldsValue([
      'backOfficeManager',
      'client',
      'agency',
      'comment',
      'creator',
      'salesManager',
      'title',
    ]),
    brand: values.brandId,
    agencyCommission: {
      value: values.agSumm,
      percent: values.agPercent,
      toRent: values.toRent,
      toNalog: values.toNalog,
      toPrint: values.toPrint,
      toMount: values.toMount,
      toAdditional: values.toAdditional,
      toNonrts: values.toNonrts,
      agent: values.akContragent,
    },
  };
}
