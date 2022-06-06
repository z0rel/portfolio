import { DatePicker } from 'antd';
import moment from 'moment';
import React from 'react';

const DROPDOWN_TOP_ALIGN = {
  points: ['bl', 'tl'],
  offset: [0, -4],
  overflow: {
    adjustX: 0,
    adjustY: 1,
  },
};

export const ReservationDatePicker = ({ value, ...props }) => {
  return (
    <DatePicker
      className="date-picker"
      suffixIcon={''}
      size={'large'}
      style={{ width: '100%' }}
      value={value ? moment(value) : null}
      dropdownAlign={DROPDOWN_TOP_ALIGN}
      format="DD.MM.YYYY"
      {...props}
    />
  );
};
