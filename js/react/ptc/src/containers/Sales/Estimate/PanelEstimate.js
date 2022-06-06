import React, { useContext, useEffect, useState } from 'react';
import styled from 'styled-components';
import { Layout, message } from 'antd';
import { EstimateContext } from './Estimate';
import { useParams } from 'react-router-dom';
import { getEstimateReservations } from './utils/EditCostsRtsReservation';
import { getRtsAdditional } from './utils/EditCostsRtsAdditional';
import { getNonRts } from './utils/EditCostsNonRTS';
import { EditCosts } from './utils/EditCosts';
import { openDeleteModal } from './utils/openDeleteModal';
import { getSidebarInfoData } from './utils/getSidebarInfoData';
import { MoveToAppendix } from './utils/MoveToAppendix';

import { DELETE_ESTIMATE_ITEM } from './q_mutations';

import { QUERY_SALES_ESTIMATE_PROTO } from './q_queries';

import Tablea from '../../../components/Tablea/Tablea_estimate';

import {
  initColumnsForPopupBookedSides,
  initColumnsForPopupExtraCharge,
  initColumnsForPopupHotPtc,
} from './estimateColumns';

import { useMutation, useQuery } from '@apollo/client';
import { ErrorComponent } from '../../../components/Logic/ErrorComponent';
import { estimate } from '../../../assets/proto_compiled/proto';
import { base64ToArrayBuffer } from '../../../components/Logic/base64ToArrayBuffer';

const PanelEstimate = ({ block, setChoosedBlock, isAppendix }) => {
  // const [activeTab, setActiveTab] = useState('reservations-rts');

  const [editingItem, setEditingItem] = useState({});
  const {
    sort,
    setSort,
    openEditModal,
    setOpenEditModal,
    periodFilter,
    createModal,
    setCreateModal,
    setPeriodFilter,
    setSidebarData,
    openPackageEditModal,
    setPackageEditModal,
    openAddToAppendix,
    setAddToAppendix,
    selectedRows,
    setSelectedRows,
  } = useContext(EstimateContext);

  let estimateQueryVariables = {};

  const params = useParams();
  if (params.id)
    estimateQueryVariables.projectId = params.id;
  if (params.appId)
    estimateQueryVariables.appendixId = params.appId;

  const { data, loading, refetch, error } = useQuery(QUERY_SALES_ESTIMATE_PROTO, { variables: estimateQueryVariables });

  const [deleteEstimateItem] = useMutation(DELETE_ESTIMATE_ITEM);
  // TODO: Delete from Appendices

  useEffect(() => {
    if (data && !loading && !error) {
      let parsedProtoData = estimate.Estimate.decode(base64ToArrayBuffer(data.searchSalesEstimateProto.content));
      setSidebarData(getSidebarInfoData(parsedProtoData));
    }
  }, [data, loading, setSidebarData, error]);

  // useEffect(() => {
  //   const refetchData = async () => {
  //     await refetch();
  //   };
  //   refetchData();
  // }, [activeTab]);

  // // TODO: сделать дефолтную сортировку РТС, НОН РТС и бронирований
  const [columnsForPopupR, setColumnsForPopupR] = useState(initColumnsForPopupBookedSides);
  const [columnsForPopupA, setColumnsForPopupA] = useState(initColumnsForPopupExtraCharge);
  const [columnsForPopupN, setColumnsForPopupN] = useState(initColumnsForPopupHotPtc);
  const [dataR, setDataR] = useState({ R: [], A: [], N: [] });

  const mappedChoosedBlock = {
    0: 'reservations-rts',
    1: 'additional-rts',
    2: 'non-rtc',
  };

  useEffect(() => {
    if (data && !loading && !error) {
      let parsedProtoData = estimate.Estimate.decode(base64ToArrayBuffer(data.searchSalesEstimateProto.content));
      let newData = {
        R: getEstimateReservations(parsedProtoData, sort, periodFilter),
        A: getRtsAdditional(parsedProtoData, sort, periodFilter),
        N: getNonRts(parsedProtoData, sort, periodFilter),
      };
      setDataR(newData);
      if (parsedProtoData.errors.length > 0)
        parsedProtoData.errors.forEach(s => message.error(s, 10))

    }
  }, [data, loading, periodFilter, sort, error]);

  let tableData = [];
  let columnsForPopup = columnsForPopupR;
  let setColumnsForPopup = setColumnsForPopupR;
  switch (mappedChoosedBlock[block]) {
    case 'reservations-rts':
      columnsForPopup = columnsForPopupR;
      setColumnsForPopup = setColumnsForPopupR;
      tableData = dataR.R;
      break;
    case 'additional-rts':
      columnsForPopup = columnsForPopupA;
      setColumnsForPopup = setColumnsForPopupA;
      tableData = dataR.A;
      break;
    case 'non-rtc':
      columnsForPopup = columnsForPopupN;
      setColumnsForPopup = setColumnsForPopupN;
      tableData = dataR.N;
      break;
    default:
      break;
  }

  const headerTableBtns = [{ title: 'ЗАБРОНИРОВАННЫЕ СТОРОНЫ' }, { title: 'ДОП. РАСХОДЫ' }, { title: 'НОН РТС' }];

  if (error)
    return <ErrorComponent error={error}/>;

  return (
    <StyledEstimatePanel>
      <Layout.Content>
        <Tablea
          columns={columnsForPopup}
          data={tableData}
          select={true}
          chooseTableBtns={headerTableBtns}
          choosedBlock={block}
          setColumns={setColumnsForPopup}
          setChoosedBlock={(blockNum) => {
            setSort('');
            setPeriodFilter('');
            setChoosedBlock(blockNum);
          }}
          edit={!isAppendix}
          del={true}
          setOpenEditModal={setOpenEditModal}
          setEditingItem={setEditingItem}
          openDeleteModal={(record) =>
            openDeleteModal({
              record,
              deleteEstimateItem,
              refetch,
              appendixId: params.appId,
            })
          }
          setSelectedItems={setSelectedRows}
          selectedItems={selectedRows}
          loading={loading}
          frontendPagination={true}
        />
      </Layout.Content>
      <EditCosts
        // редактирование расхода
        openModal={openEditModal}
        setOpenModal={setOpenEditModal}
        editingItem={editingItem}
        block={mappedChoosedBlock[block]}
        blocki={block}
        refetch={refetch}
        selectedRows={selectedRows}
      />
      <EditCosts
        // Создание расхода
        openModal={createModal}
        setOpenModal={setCreateModal}
        editingItem={undefined}
        block={mappedChoosedBlock[block]}
        blocki={block}
        refetch={refetch}
        isEditing={false}
        selectedRows={selectedRows}
      />
      <EditCosts
        // Пакетное изменение
        openModal={openPackageEditModal}
        setOpenModal={setPackageEditModal}
        editingItem={undefined}
        block={mappedChoosedBlock[block]}
        blocki={block}
        refetch={refetch}
        isEditing={false}
        isPackage={true}
        selectedRows={selectedRows}
      />
      {params.id && (
        <MoveToAppendix
          // Добавление в смету приложения
          openModal={openAddToAppendix}
          setOpenModal={setAddToAppendix}
          selectedRows={selectedRows}
          refetch={refetch}
          projectId={params.id}
          mappedChoosedBlock={mappedChoosedBlock}
          block={block}
        />
      )}
    </StyledEstimatePanel>
  );
};

const StyledEstimatePanel = styled.div`
  .ant-drawer-bottom .ant-drawer-content-wrapper {
    left: 60px;
    box-shadow: 0px -4px 16px rgba(0, 0, 0, 0.0973558) !important;
    border-radius: 8px 8px 0px 0px;
    width: calc(100% - 60px);
  }

  .ant-drawer-bottom .ant-drawer-close {
    padding: 10px 24px;
  }
  width: 100%;
  overflow-x: hidden;
`;

export default PanelEstimate;
