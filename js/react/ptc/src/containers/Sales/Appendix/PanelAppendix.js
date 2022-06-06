import React from 'react';
import Tablea from '../../../components/Tablea/Tablea_estimate';
import { column } from '../../../components/Table/utils'


const columns = [
  column('Город', 'city', 100),
  column('Формат', 'format', 100),
  column('Период', 'period', 100),
  column('Аренда', 'renta', 100),
  column('Скидка', 'rentaDiscount', 100),
  column('Аренда со скидкой', 'rentaWithDiscount', 100),
  column('Печать', 'print', 100),
  column('Монтаж', 'install', 100),
  column('Доп. работы', 'addexpense', 100),
  column('Налог', 'nalog', 100),
  column('Скидка на налог', 'nalogDiscount', 100),
  column('Налог со скидкой', 'nalogWithDiscount', 100),
  column('Общая Сумма', 'amount', 100),
];


export const PanelAppendix = ({ tableData, onExportClick, loading }) => {

  return (
    <>
      <div className="outdoor-table-bar">
        <Tablea
          title="Адресная программа"
          // changeColumns={changeColumns}
          columns={columns}
          data={tableData}
          select={false}
          edit={false}
          loading={loading}
          notheader={false}
          onExportClick={onExportClick}
        />
      </div>

      <style>
        {
          `.outdoor-table-bar {
            width: 100%;
            overflow-x: hidden;
          }`
        }
      </style>
    </>
  );
};

