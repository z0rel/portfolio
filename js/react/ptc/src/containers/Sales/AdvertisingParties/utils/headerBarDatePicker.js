import React, { useContext, useState } from 'react';
import { DatePicker, Select } from 'antd';
import { adverContext } from '../AdvertisingParties';
import nextIcon from '../../../../img/TableHeader/next.svg';
import { currentPeriod, nextDate, periodChange, pickerTitle, prevDate } from './dateSwitches';

const HeaderBarDatePicker = () => {
  const { Option } = Select;
  const [displayedPeriod, setDisplayedPeriod] = useState('month');
  const { setPeriod, period } = useContext(adverContext);
  return (
    <div
      style={{
        marginLeft: 20,
      }}
    >
      <div>
        <img
          alt=""
          src={nextIcon}
          style={{
            transform: 'rotate(180deg)',
            marginRight: 5,
            cursor: 'pointer',
          }}
          onClick={() => {
            setPeriod((prevState) => prevDate(prevState, displayedPeriod));
          }}
        />
      </div>
      <Select
        style={{
          borderRadius: 4,
          marginRight: -5,
          width: 95,
        }}
        value={displayedPeriod}
        onChange={(value) => {
          setDisplayedPeriod(value);
          currentPeriod(value, setPeriod);
        }}
      >
        <Option value="week">Неделя</Option>
        <Option value="month">Месяц</Option>
        <Option value="year">Год</Option>
      </Select>
      <div
        style={{
          position: 'relative',
        }}
      >
        <DatePicker
          picker={displayedPeriod}
          format=" "
          inputReadOnly
          style={{
            minWidth: 220,
            borderRadius: 4,
          }}
          onSelect={(date) => periodChange(displayedPeriod, date, setPeriod)}
          placeholder=""
        />
        <span
          style={{
            position: 'absolute',
            zIndex: 1,
            color: '#000',
            left: '-7px',
            transform: 'translateY(40%)',
            fontSize: 12,
            textAlign: 'center',
            width: '100%',
            pointerEvents: 'none',
          }}
        >
          {pickerTitle(period.start, period.end)}
        </span>
      </div>
      <div>
        <img
          alt=""
          src={nextIcon}
          style={{
            marginLeft: 5,
            cursor: 'pointer',
          }}
          onClick={() => {
            setPeriod((prevState) => nextDate(prevState, displayedPeriod));
          }}
        />
      </div>
    </div>
  );
};

export default HeaderBarDatePicker;
