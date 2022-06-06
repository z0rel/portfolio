import moment from 'moment';

export const prevDate = (prevState, displayedPeriod) => {
  const start = moment(prevState.start);
  const end = moment(prevState.end);
  switch (displayedPeriod) {
    case 'week':
      return {
        start: start.subtract(1, 'weeks').toDate(),
        end: end.subtract(1, 'weeks').toDate(),
      };
    case 'month':
      return {
        start: start.subtract(1, 'months').toDate(),
        end: end.subtract(1, 'months').toDate(),
      };
    case 'year':
      return {
        start: start.subtract(1, 'years').toDate(),
        end: end.subtract(1, 'years').toDate(),
      };
    default:
      return prevState;
  }
};

export const nextDate = (prevState, displayedPeriod) => {
  const start = moment(prevState.start);
  const end = moment(prevState.end);
  switch (displayedPeriod) {
    case 'week':
      return {
        start: start.add(1, 'weeks').toDate(),
        end: end.add(1, 'weeks').toDate(),
      };
    case 'month':
      return {
        start: start.add(1, 'months').toDate(),
        end: end.add(1, 'months').toDate(),
      };
    case 'year':
      return {
        start: start.add(1, 'years').toDate(),
        end: end.add(1, 'years').toDate(),
      };
    default:
      return prevState;
  }
};

export const currentPeriod = (value, setPeriod) => {
  const date = moment();
  switch (value) {
    case 'week':
      {
        const weekStart = date.isoWeekday(1).set({ hour: 0, minute: 0, second: 0 }).toDate();
        const weekEnd = date.isoWeekday(7).set({ hour: 0, minute: 0, second: 0 }).toDate();
        setPeriod({ start: weekStart, end: weekEnd });
      }
      break;
    case 'month':
      {
        const monthStart = date.startOf('month').set({ hour: 0, minute: 0, second: 0 }).toDate();
        const monthEnd = date.endOf('month').set({ hour: 0, minute: 0, second: 0 }).toDate();
        setPeriod({ start: monthStart, end: monthEnd });
      }
      break;
    case 'year':
      {
        const yearStart = date.startOf('year').set({ hour: 0, minute: 0, second: 0 }).toDate();
        const yearEnd = date.endOf('year').set({ hour: 0, minute: 0, second: 0 }).toDate();
        setPeriod({ start: yearStart, end: yearEnd });
      }
      break;
    default:
      return;
  }
};

export const periodChange = (displayedPeriod, date, setPeriod) => {
  switch (displayedPeriod) {
    case 'week':
      {
        const weekStart = date.isoWeekday(1).set({ hour: 0, minute: 0, second: 0 }).toDate();
        const weekEnd = date.isoWeekday(7).set({ hour: 0, minute: 0, second: 0 }).toDate();
        setPeriod({ start: weekStart, end: weekEnd });
      }
      break;
    case 'month':
      {
        const monthStart = date.startOf('month').set({ hour: 0, minute: 0, second: 0 }).toDate();
        const monthEnd = date.endOf('month').set({ hour: 0, minute: 0, second: 0 }).toDate();
        setPeriod({ start: monthStart, end: monthEnd });
      }
      break;
    case 'year':
      {
        const yearStart = date.startOf('year').set({ hour: 0, minute: 0, second: 0 }).toDate();
        const yearEnd = date.endOf('year').set({ hour: 0, minute: 0, second: 0 }).toDate();
        setPeriod({ start: yearStart, end: yearEnd });
      }
      break;
    default:
      return;
  }
};

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

export const pickerTitle = (start, end) => {
  let monthFirst = MONTHS[start.getMonth()];
  let monthNext = MONTHS[end.getMonth()];
  let monthDayFirst = start.toLocaleString('ru-RU', { day: 'numeric' });
  let monthDayNext = end.toLocaleString('ru-RU', { day: 'numeric' });
  return `${monthDayFirst} ${monthFirst}  – ${monthDayNext} ${monthNext} ${end.getFullYear()}`;
};
