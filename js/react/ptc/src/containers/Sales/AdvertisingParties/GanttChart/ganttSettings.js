import { mappedGanttColumnsStart, mappedGanttColumnsForPopup } from '../utils/infoColumnsInitialData';

const ganttSettings = (period, gridAndChartWidths, setWidthsGridAndChart, orderBy, setOrderBy) => ({
  // currentTime: new Date(year, month, 2, 12, 0, 0),
  // Optionally, initialize custom theme and templates (themes.js, templates.js).
  // initializeGanttChartTheme(settings, theme);
  // initializeGanttChartTemplates(settings, theme);
  // Set up continuous schedule (24/7).
  columnOrderBy: orderBy,
  columnSetOrderBy: setOrderBy,
  workingWeekStart: 0, // Sunday
  workingWeekFinish: 6, // Saturday
  visibleDayStart: 0, // 00:00
  visibleDayFinish: 24 * 60 * 60 * 1000, // 24:00
  timelineStart: period.start,
  timelineFinish: period.end,
  displayedTime: period.start,
  updateScale: 24 * 60 * 60 * 1000,
  hourWidth: 2.5,
  barCornerRadius: 6,
  daysOfWeek: ['Вс', 'Пн', 'Вт', 'Ср', 'Чт', 'Пт', 'Сб'],
  weekStartDay: 1,
  headerHeight: 26 * 2,
  barHeight: 25,
  barMargin: 10,
  itemHeight: 40,
  allowUserToResizeColumns: true,
  isRelativeToTimezone: false,
  horizontalGridLines: '#D3DFF0',
  isBaselineVisible: false,
  isTaskCompletedEffortVisible: false,
  isMouseWheelZoomEnabled: false,
  selectionMode: 'ExtendedFocus',
  headerBackground: '#FFFFFF',
  isGridVisible: true,
  gridWidth: gridAndChartWidths ? gridAndChartWidths[0] : '20%',
  chartWidth: gridAndChartWidths ? gridAndChartWidths[1] : '80%',
  splitterWidth: 4,
  columnWidthChangeHandler: (column, width) => {
    let obj1 = mappedGanttColumnsStart.get(column.dataIndex);
    let obj2 = mappedGanttColumnsForPopup.get(column.dataIndex);
    if (obj1)
      obj1.width = width;
    if (obj2)
      obj2.width = width;
  },
  splitterPositionChangeHandler: (gridWidth, chartWidth) => {
    if (setWidthsGridAndChart) {
      let sumval = gridWidth + chartWidth;
      let gridPercent = Math.round(100.0 * gridWidth / sumval);
      let chartPercent = Math.round(100.0 * chartWidth / sumval);
      setWidthsGridAndChart([`${gridPercent}%`, `${chartPercent}%`])
    }
  },
  isTaskToolTipVisible: false,
  scales: [
    {
      scaleType: 'NonworkingTime',
      isHeaderVisible: false,
      isHighlightingVisible: true,
      highlightingStyle: 'stroke-width: 0; fill: #f8f8f8',
    },
    {
      scaleType: 'Weeks',
      headerStyle: 'padding: 2.25px; border-right: solid 1px #D3DFF0;',
      headerTextFormat: (item) => {
        const MONTHS = [
          'января',
          'февраля',
          'марта',
          'апреля',
          'мая',
          'июня',
          'июля',
          'августа',
          'сентября',
          'октября',
          'ноября',
          'декабря',
        ];
        let nextDate = new Date(item);
        nextDate.setDate(item.getDate() + 6);
        let monthFirst = MONTHS[item.getMonth()];
        let monthNext = MONTHS[nextDate.getMonth()];
        let monthDayFirst = item.toLocaleString('ru-RU', { day: 'numeric' });
        let monthDayNext = nextDate.toLocaleString('ru-RU', { day: 'numeric' });
        return `${monthDayFirst} ${monthFirst} ${item.getFullYear()} – ${monthDayNext} ${monthNext} ${nextDate.getFullYear()}`;
      },
    },
    // {
    //   scaleType: 'Days',
    //   headerTextFormat: 'DayOfWeek',
    //   intervals: () => {
    //     const daysOfWeek = ['Пн', 'Вт', 'Ср', 'Чт', 'Пт', 'Сб', 'Вс'];
    //     const intervals = [];
    //     const dayDuration = 24 * 60 * 60 * 1000;
    //     for (let d = period.start; d < period.end; d = new Date(d.valueOf() + dayDuration)) {
    //       intervals.push({
    //         headerText: daysOfWeek[d.getUTCDate()],
    //         start: d,
    //         finish: new Date(d.valueOf() + dayDuration),
    //       });
    //     }
    //     return intervals;
    //   },
    //   headerStyle: 'padding: 2.25px; border-right: solid 1px #D3DFF0',
    // },
    {
      scaleType: 'Days',
      headerTextFormat: 'DayOfWeek',
      headerStyle: 'padding: 2.25px; border-right: solid 1px #D3DFF0',
    },
    // {
    //   scaleType: 'CurrentTime',
    //   isHeaderVisible: false,
    //   isSeparatorVisible: true,
    //   separatorStyle: 'stroke: #8bbf8a; stroke-width: 0.5px',
    // },
  ],
});

export default ganttSettings;
