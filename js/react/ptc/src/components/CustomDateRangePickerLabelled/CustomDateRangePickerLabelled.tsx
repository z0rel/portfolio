import React from 'react';
import { CustomDateRangePicker, CustomDateRangePickerProps } from '../CustomDateRangePicker/CustomDateRangePicker';
import { FormItem } from '../Form/FormItem';

type CustomDateRangePickerLabelledProps = {
  // Реальная ширина окна
  widthWindow: number;
  // Ширина окна при которой меняется placeholder
  widthBreak: number;
  // Короткие placehloder
  placeholdersShort: string[];
  // Длинные placehloder
  placeholdersBig: string[];
} & CustomDateRangePickerProps;

const PLACEHOLDERS_SHORT = ['Начало', 'Окончание'];
const PLACEHOLDERS_BIG = ['Дата начала', 'Дата окончания'];

function CustomDateRangePickerLabelled({
  className,
  name,
  widthWindow = 1,
  widthBreak = 0,
  placeholdersShort = PLACEHOLDERS_SHORT,
  placeholdersBig = PLACEHOLDERS_BIG,
  ...props
}: CustomDateRangePickerLabelledProps): React.ReactElement {
  return (
    <div className={className}>
      <FormItem
        name={name}
        required={true}
        rules={{ message: 'Пожалуйста укажите период.', required: true }}
        labelWrapper={
          <>
            <div className="period-title">
              <p className="formItem-title">Дата начала</p>
              <p className="formItem-title">Дата окончания</p>
            </div>
          </>
        }
        className="form-item-period"
      >
        <CustomDateRangePicker
          className="date-picker"
          placeholder={[
            widthWindow > widthBreak ? placeholdersBig[0] : placeholdersShort[0],
            widthWindow > widthBreak ? placeholdersBig[1] : placeholdersShort[1],
          ]}
          {...props}
        />
      </FormItem>
    </div>
  );
}

export type { CustomDateRangePickerLabelledProps };
export { CustomDateRangePickerLabelled };
