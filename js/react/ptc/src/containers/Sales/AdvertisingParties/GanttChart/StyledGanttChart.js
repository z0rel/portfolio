import React, { useEffect, useRef } from 'react';
import '../styles/styles_adv_part.scss';
import { ScheduleChartView } from './dlh/ScheduleChartView';

export const ScheduleChartView1 = function({ style, items, settings, columns, setChangedItems }) {
  let ref = useRef(null);
  if (!ref)
    ref = React.createRef();
  let dl_columns = ScheduleChartView.getDefaultColumns(items, settings);
  let copied_settings = { ...settings };
  if (columns) {
    for (let col of columns) {
      if (col.isShowed) {
        dl_columns.push(col);
      }
    }
    copied_settings.columns = dl_columns;
  }

  useEffect(() => {
    if (ref.current && items) {
      ScheduleChartView.initialize(ref.current, items, copied_settings);
      copied_settings.itemPropertyChangeHandler = function(item, propertyName, isDirect, isFinal) {
        if (isDirect && isFinal && propertyName !== 'isSelected' && propertyName !== 'isExpanded') {
          setChangedItems((prevState) => {
            const changedItems = prevState.filter((reservation) => reservation.id !== item.content);
            return [
              ...changedItems,
              {
                id: item.content,
                dateFrom: item.start,
                dateTo: item.finish,
              },
            ];
          });
        }
      };
    }
  });

  return <div ref={ref} style={style}></div>;
};
