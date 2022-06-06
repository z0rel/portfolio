import { advertising_sides } from '../../../../assets/proto_compiled/proto';
import { getProjectCodeWithTitle } from '../../../../components/Logic/projectCode';
import { getConstructionSideCodeByArgs } from '../../../../components/Logic/constructionSideCode';
import { base64ToArrayBuffer } from '../../../../components/Logic/base64ToArrayBuffer';

const mapData = (content, history) => {
  let mappedData = [];

  let getBarClass = (barClass) => {
    if (barClass === 'Свободно')
      return 'gantt-bar-status-free';
    if (barClass === 'Забронировано')
      return 'gantt-bar-status-reserved';
    if (barClass === 'Утверждено')
      return 'gantt-bar-status-approved';
    if (barClass === 'Продано')
      return 'gantt-bar-status-saled';
    if (barClass === 'unavailable')
      return 'gantt-bar-status-unavailable';
    return 'gantt-bar-status-reserved';
  };

  let getBarTitle = (barClass) => {
    if (barClass === 'Свободно')
      return 'cвободно';
    if (barClass === 'Забронировано')
      return 'забронировано';
    if (barClass === 'Утверждено')
      return 'утверждено';
    if (barClass === 'Продано')
      return 'продано';
    if (barClass === 'unavailable')
      return 'недоступно';
    return 'забронировано';
  };

  // TODO: сделать более умное задание даты начала
  let allReservationsStartDate = new Date(2020, 1, 1, 0, 0, 0);
  let allReservationsEndDate = new Date(2021, 1, 1, 0, 0, 0);

  if (content) {
    let sides = advertising_sides.ConstructionSides.decode(base64ToArrayBuffer(content));
    for (let side of sides.constructionSide) {
      mappedData.push({
        content: side.id,
        start: allReservationsStartDate,
        finish: allReservationsEndDate,
        code: getConstructionSideCodeByArgs(
          side.postcodeTitle,
          side.numInDistrict,
          side.formatCode,
          side.sideCode,
          side.advertisingSideCode,
        ),
        format: side.formatTitle,
        city: side.cityTitle || '',
        district: side.districtTitle,
        marketingAddress: side.marketingAddress,
        side: side.advertisingSideTitle,
        size: side.size,
        owner: side.isNonRts === true ? side.ownerTitle : side.isNonRts === false ? 'РТС' : '',
        package: side.package,
        status: side.statusConnection ? 'Да' : 'Нет',
        ganttChartItems: side.reservations.map((reservation) => ({
          content: reservation.id,
          start: reservation.dateFrom,
          finish: reservation.dateTo,
          barClass: getBarClass(reservation.reservationTypeTitle),
          project: getProjectCodeWithTitle(
            reservation.projectCreatedAt,
            reservation.projectNumInYear,
            reservation.projectTitle,
          ),
          projectId: reservation.projectId,
          comment: reservation.projectComment,
          status: side.statusConnection ? 'Да' : 'Нет',
          package: side.package,
          salesManager: reservation.salesManagerLastName + ' ' + reservation.salesManagerFirstName,
          backOfficeManager: reservation.backOfficeManagerLastName + ' ' + reservation.backOfficeManagerFirstName,
          type: reservation.reservationTypeTitle,
          textValue: (reservation.brandTitle || '') + ' - ' + getBarTitle(reservation.reservationTypeTitle),
          branding: reservation.branding,
          history: history,
        })),
      });
    }
  }

  return mappedData;
};

export default mapData;
