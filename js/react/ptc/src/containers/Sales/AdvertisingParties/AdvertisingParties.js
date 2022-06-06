import React, { createContext, useState } from 'react';
import { Layout, message } from 'antd';
import { useHistory, useParams } from 'react-router';
import moment from 'moment';
import { HeaderTitleWrapper, HeaderWrapper, StyledButton } from '../../../components/Styles/DesignList/styles';
import { TitleLogo } from '../../../components/Styles/ComponentsStyles';
import { JobTitle } from '../../../components/Styles/StyledBlocks';
import { ButtonGroup } from '../../../components/Styles/ButtonStyles';
import { BreadCrumbsRoutes } from '../../../components/BreadCrumbs/BreadCrumbs';
import { CreateReservationsSlider } from './BottomSliderCreateReservations';
import FilterBar from './FilterBar';
import { FilterLeftBar } from './LeftBarFilters/FilterLeftBar';
import Pagination from './GanttChart/pagination';
import updateReservation, { UPDATE_MUTATION } from './utils/updateReservation';
import { useMutation, useQuery } from '@apollo/client';
import { colorAccent, colorOrangeAccent } from '../../../components/Styles/Colors';
import { GanttChartAdvertisingSides } from './GanttChartAdvertisingSides';
import { routes } from '../../../routes';
import HeaderBar from '../../../components/HeaderBar/HeaderBarTablea';
import { ganttColumns, ganttColumnsForPupup } from './utils/infoColumnsInitialData';
import HeaderBarDatePicker from './utils/headerBarDatePicker';
import PageLoading from './GanttChart/pageLoading';
import { SEARCH_CONSTRUCTION_SIDE_WITH_RESERVATION } from './q_queries';

import './styles/styles_adv_part.scss';
import {
  AdvertisingPartiesProjectCardBreadCrumbs,
  AdvertisingPartiesProjectCardTitle,
} from './utils/titlesForProjectCard';

export const adverContext = createContext();

const mapOrderByArr = (orderBy) => {
  return orderBy.map(({ asc, idx, specs }) => specs.map((col) => (asc ? col : `_${col}`))).flat();
};

const AdvertisingParties = () => {
  const [collapsed, setCollapsed] = useState(true);
  const [chartItems, setChartItems] = useState([]);
  const [filter, setFilter] = useState({});
  const [refetch, setRefetch] = useState(null);
  const [ganttUpdater, setGanttUpdater] = useState(null);
  const [infoColumns, setInfoColumns] = useState(ganttColumns);
  const [changedItems, setChangedItems] = useState([]);
  const [offset, setOffset] = useState({
    offset: 0,
    limit: 25,
  });
  const [pageLoading, setPageLoading] = useState(false);
  const [itemsLength, setItemsLength] = useState(0);

  const [sliderAddShowed, setSliderAddShowed] = useState(false);
  const [period, setPeriod] = useState(() => {
    const date = moment();
    const start = date.startOf('month').toDate();
    const end = date.endOf('month').toDate();
    return {
      start,
      end,
    };
  });

  let dstFilter = filter ? filter.dstFilter : {};
  let [orderBy, setOrderBy] = useState([]);

  let searchQuery = useQuery(SEARCH_CONSTRUCTION_SIDE_WITH_RESERVATION, {
    variables: {
      ...dstFilter,
      orderBy: mapOrderByArr(orderBy),
      offset: 0,
      limit: 25,
    },
  });

  const history = useHistory();

  const [updateRes] = useMutation(UPDATE_MUTATION);

  const getSelected = (items) => {
    let dst = [];
    for (let item of items) {
      if (item.isSelected) {
        dst.push(item.content);
      }
    }
    return dst;
  };

  const isProjectSelector = window.location.pathname.startsWith(
    routes.sales.project_card_advertising_parties.startPrefix,
  );
  const urlParams = useParams();

  const jobTitleLocal = isProjectSelector ? (
    <AdvertisingPartiesProjectCardTitle id={urlParams?.id}/>
  ) : (
    <JobTitle>Справочник рекламных сторон</JobTitle>
  );

  const routesBreadcrumbs = isProjectSelector ? (
    <AdvertisingPartiesProjectCardBreadCrumbs id={urlParams?.id}/>
  ) : (
    <BreadCrumbsRoutes links={[routes.sales.root, routes.sales.advertising_parties]}/>
  );

  return (
    <adverContext.Provider
      value={{
        filter,
        setFilter,
        chartItems,
        setChartItems,
        refetch,
        setRefetch,
        period,
        setPeriod,
        infoColumns,
        changedItems,
        setChangedItems,
        offset,
        sliderAddShowed,
        setSliderAddShowed,
        setPageLoading,
        searchQuery,
        orderBy,
        setOrderBy,
      }}
    >
      <Layout>
        <Layout
          style={{
            overflow: 'hidden',
            position: 'relative',
          }}
        >
          <FilterLeftBar setCollapsed={setCollapsed} collapsed={collapsed}/>
          {collapsed && <FilterBar refetch={refetch} ganttUpdater={ganttUpdater}/>}
          <Layout className="layout-main" style={{ padding: '30px 30px 0 30px' }}>
            {routesBreadcrumbs}
            <HeaderWrapper>
              <HeaderTitleWrapper>
                <TitleLogo/>
                {jobTitleLocal}
              </HeaderTitleWrapper>
              <ButtonGroup>
                <StyledButton
                  backgroundColor={colorAccent}
                  onClick={() => {
                    const selected = getSelected(chartItems);
                    if (selected.length) {
                      setSliderAddShowed(true);
                    }
                    else {
                      message.error('Пожалуйста сначала выберите сторону конструкции');
                    }
                  }}
                >
                  Быстрая бронь
                </StyledButton>
                <StyledButton
                  backgroundColor={colorAccent}
                  onClick={() => {
                    history.push(routes.sales.project_new.path);
                  }}
                >
                  Создать проект
                </StyledButton>
                <StyledButton
                  backgroundColor={colorAccent}
                  onClick={() => {
                    console.log(changedItems);
                    updateReservation(updateRes, changedItems, setPageLoading);
                  }}
                >
                  Сохранить
                </StyledButton>
                <StyledButton backgroundColor={colorOrangeAccent}>Создать отчет</StyledButton>
              </ButtonGroup>
            </HeaderWrapper>
            <div style={{ display: 'flex', flexDirection: 'column' }}>
              {itemsLength <= 0 ? (
                ''
              ) : (
                <HeaderBar
                  columnsConfig={{
                    columnsForPopup: ganttColumnsForPupup,
                    setColumnsForPopup: setInfoColumns,
                  }}
                  enableEditQuantityOfColumns
                >
                  <HeaderBarDatePicker/>
                </HeaderBar>
              )}
              <div className="outdoor-table-bar" style={{ flex: '0 1 auto' }}>
                <div
                  style={{
                    position: 'relative',
                  }}
                >
                  <GanttChartAdvertisingSides
                    setItemsLength={setItemsLength}
                    offset={offset}
                    filter={filter}
                    setPageLoading={setPageLoading}
                    setGanttUpdater={setGanttUpdater}
                  />
                  {pageLoading ? <PageLoading/> : ''}
                </div>
                {itemsLength > 0 ? <Pagination length={itemsLength} setOffset={setOffset}/> : ''}
              </div>
              {sliderAddShowed && <CreateReservationsSlider/>}
            </div>
          </Layout>
        </Layout>
      </Layout>
    </adverContext.Provider>
  );
};
export default AdvertisingParties;
