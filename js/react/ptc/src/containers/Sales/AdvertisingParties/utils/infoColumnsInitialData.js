const UP_CLASS = (active) => `anticon anticon-caret-up ant-table-column-sorter-up ${active ? 'active' : ''}`;
const DOWN_CLASS = (active) => `anticon anticon-caret-down ant-table-column-sorter-down ${active ? 'active' : ''}`;

const IDX_NOSORT = 0;
const IDX_UP = 1;
const IDX_DOWN = 2;

const ganttColumn = (header, width, dataIndex, isShowed, orderBySpec) => ({
  header: header,
  width: width,
  dataIndex: dataIndex,
  isShowed: isShowed,
  cellTemplate: (item) => item.scheduleChartView.ownerDocument.createTextNode(item[dataIndex]),
  allowUserToResize: true,
  orderState: 0,
  clickHandler: (event, self, settings, upSpan, downSpan) => {
    let orderByArr = settings.columnOrderBy;

    self.orderState += 1;
    if (self.orderState > 2)
      self.orderState = IDX_NOSORT;

    if (self.orderState === IDX_NOSORT) {
      upSpan.setAttribute('class', UP_CLASS(false));
      downSpan.setAttribute('class', DOWN_CLASS(false));
    }
    else if (self.orderState === IDX_UP) {
      upSpan.setAttribute('class', UP_CLASS(true));
      downSpan.setAttribute('class', DOWN_CLASS(false));
    }
    else if (self.orderState === IDX_DOWN) {
      upSpan.setAttribute('class', UP_CLASS(false));
      downSpan.setAttribute('class', DOWN_CLASS(true));
    }

    if (orderByArr && orderBySpec && orderBySpec.length > 0) {
      if (self.orderState === IDX_NOSORT) {
        let filteredOrderByArr = orderByArr.filter((x) => !(x.idx === self.dataIndex));
        settings.columnSetOrderBy(filteredOrderByArr);
      }
      else if (self.orderState === IDX_UP) {
        let filteredOrderByArr = orderByArr.filter((x) => !(x.asc && x.idx === self.dataIndex));
        filteredOrderByArr.push({ asc: true, idx: self.dataIndex, specs: orderBySpec });
        settings.columnSetOrderBy(filteredOrderByArr);
      }
      else if (self.orderState === IDX_DOWN) {
        let filteredOrderByArr = orderByArr.filter((x) => !(!x.asc && x.idx === self.dataIndex));
        filteredOrderByArr.push({ asc: false, idx: self.dataIndex, specs: orderBySpec });
        settings.columnSetOrderBy(filteredOrderByArr);
      }
    }
  },
});

export const ganttColumns = [
  ganttColumn('Код', 200, 'code', true, [
    'postcodeTitle',
    'numInDistrict',
    'formatCode',
    'sideCode',
    'advertisingSideCode',
  ]),
  ganttColumn('Формат', 200, 'format', true, ['formatTitle']),
  ganttColumn('Город', 130, 'city', true, ['cityTitle']),
];

export const ganttColumnsForPupup = [
  ganttColumn('Код', 200, 'code', true, [
    'postcodeTitle',
    'numInDistrict',
    'formatCode',
    'sideCode',
    'advertisingSideCode',
  ]),
  ganttColumn('Формат', 200, 'format', true, ['formatTitle']),
  ganttColumn('Город', 130, 'city', true, ['cityTitle']),
  ganttColumn('Район', 130, 'district', false, ['cityTitle']),
  ganttColumn('Маркетинговый адрес', 450, 'marketingAddress', false, ['marketingAddress']),
  ganttColumn('Сторона', 130, 'side', false, ['sideTitle']),
  ganttColumn('Размер', 130, 'size', false, ['sideSize']),
  ganttColumn('Владелец', 130, 'owner', false, ['isNonrts', 'nonrtsOwnerTitle']),
  ganttColumn('Пакет', 130, 'package', false, ['packageTitle']),
  ganttColumn('Освещение', 130, 'status', false, ['statusConnection']),
];

export const mappedGanttColumnsStart = new Map(ganttColumns.map((item) => [item.dataIndex, item]));
export const mappedGanttColumnsForPopup = new Map(ganttColumnsForPupup.map((item) => [item.dataIndex, item]));
