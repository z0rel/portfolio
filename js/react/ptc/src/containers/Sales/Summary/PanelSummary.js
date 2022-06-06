import React from 'react';
import Tablea from '../../../components/Tablea/Tablea_estimate';


const PanelDesign = () => {
  const columns = [
    { title: 'Город', dataIndex: 'city', width: 100, },
    { title: 'Формат', dataIndex: 'format', width: 100, },
    { title: 'Назначение стороны', dataIndex: 'siderole', width: 100, },
    { title: 'Адрес', dataIndex: 'address', width: 100, },
    { title: 'Монтаж', dataIndex: 'install', width: 100, },
    { title: 'Фотоотчет', dataIndex: 'photo', width: 100, },
    { title: 'Доп. Фотоотчет', dataIndex: 'addphoto', width: 100, },
    { title: 'Смета', dataIndex: 'estimate', width: 100, },
    { title: 'Приложение', dataIndex: 'app', width: 100, },
    { title: 'Счет', dataIndex: 'invoice', width: 100, },
    { title: 'АВР', dataIndex: 'avr', width: 60, },
  ];
  const data = [];
  return (
    <>
      <div className="outdoor-table-bar">
        <Tablea style={{ width: '100%' }} columns={columns} data={data} select={false} />
      </div>

      <style>
        {`.outdoor-table-bar {
            width: 100%;
          }
          .design-info {
            border-radius: 8px;
            border: 1px solid #d3dff0;
            // height: 100%;
            // padding: 1.5%;
            // flex: 0 1 30vw;
            // margin: 0 2vw 0 0;
          }`}
      </style>
    </>
  );
};

export default PanelDesign;
