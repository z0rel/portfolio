import React, { useEffect, useState } from 'react';
import { useHistory } from 'react-router-dom';
import { gql, useMutation } from '@apollo/client';
import {
  columnOrderByH,
  columnOrderByHList,
  columnOrderByS,
  columnOrderBySList,
} from '../../../components/Table/utils';
import { routes } from '../../../routes';
import { Tablea } from '../../../components/Tablea/Tablea_estimate';
import deleteModal from './utils/deleteModal';
import { ErrorComponent } from '../../../components/Logic/ErrorComponent';
import { onChangeOrderBy, onChangePage } from '../../../components/Tablea/relayPagination';

const columnsOfReservation = [
  // columnSortedNum('Номер', 'id_mapped', 130),
  columnOrderBySList('Код стороны', 'reservation_code', 130, [
    'postcode_title',
    'num_in_district',
    'format_code',
    'side_code',
    'adv_side_code',
  ]),
  columnOrderByS('Город', 'reservation_city', 100),
  columnOrderByS('Адрес', 'reservation_address', 100),
  columnOrderByS('Формат', 'reservation_format', 100),
  columnOrderByH('Сторона', 'reservation_side', 100),
  columnOrderByS('Дата создания', 'reservation_creationDate', 100),
  columnOrderByS('Дата начала', 'reservation_startDate', 100),
  columnOrderByS('Дата окончания', 'reservation_expirationDate', 100),
  columnOrderByH('Статус', 'reservation_status', 100),
  columnOrderByHList('Продление брони', 'reservation_renewalOfReservation', 100, []),
  columnOrderByS('Брендирование', 'reservation_branding', 100),
  columnOrderByH('Освещение', 'reservation_lighting', 100),
  columnOrderByH('Пакет', 'reservation_package', 100),
  columnOrderByHList('Дизайн', 'reservation_design', 100, []),
  // columnSorted('ID стороны', 'side_id', 100, false),
];

const columnsOfAppendix = [
  columnOrderBySList('Номер приложения', 'appendix_code', 130, ['code']),
  columnOrderBySList('Сумма', 'appendix_summa', 100, []),
  columnOrderBySList('Дата создания', 'appendix_createDate', 100, ['created_date']),
  columnOrderBySList('Сроки', 'appendix_reservDates', 220, ['period_start_date', 'period_end_date']),
];

export const IDX_BLOCK_RESERVATIONS = 0;

export const PanelProjectCard = ({
                                   choosedBlock,
                                   setChoosedBlock,
                                   panelData,
                                   setSelectedItems,
                                   startEditItemAppendix,
                                   startEditItemReservation,
                                   onFastSearchSearch
                                 }) => {
  const history = useHistory();

  const DELETE_RESERVATION = gql`
    mutation deleteReservation($id: ID!) {
      deleteReservation(id: $id) {
        found
      }
    }
  `;

  const [deleteReservation] = useMutation(DELETE_RESERVATION);
  const [columns, setColumns] = useState(columnsOfReservation);
  const [data, setData] = useState(panelData.reservations);

  useEffect(() => {
    if (choosedBlock === IDX_BLOCK_RESERVATIONS) {
      setColumns(columnsOfReservation);
      setData(panelData.reservations);
    }
    else {
      setColumns(columnsOfAppendix);
      setData(panelData.appendices);
    }
  }, [choosedBlock, panelData.appendices, panelData.reservations]);

  const headerTableBtns = [{ title: 'ЗАБРОНИРОВАННЫЕ СТОРОНЫ' }, { title: 'ПРИЛОЖЕНИЯ' }];

  const onRow = (record) => ({
    onDoubleClick: () => history.push(routes.sales.appendix.url(record.id)),
  });
  const setEditingItem = (record) => {
    if (choosedBlock === IDX_BLOCK_RESERVATIONS) {
      startEditItemReservation(record);
    }
    else {
      startEditItemAppendix(record);
    }
  };

  if (data.error)
    return <ErrorComponent error={data.error}/>;

  return (
    <Tablea
      columns={columns}
      data={data.mappedData}
      select={true}
      chooseTableBtns={headerTableBtns}
      loading={data.loading}
      openDeleteModal={(record, block) => deleteModal(record, block, data.refetch, deleteReservation)}
      setEditingItem={setEditingItem}
      setColumns={setColumns}
      choosedBlock={choosedBlock}
      setChoosedBlock={setChoosedBlock}
      edit={true}
      del={true}
      setSelectedItems={setSelectedItems}
      totalCount={data.totalCount}
      onRow={onRow}
      onChangePage={onChangePage(data.setPageInfo)}
      paginationRelayApi
      onChangeOrderBy={(columns) => onChangeOrderBy(columns, data.setOrderBy, data.refetch)}
      onFastSearchSearch={onFastSearchSearch}
    />
  );
};
