import React, { useContext, useEffect } from 'react';
import { useHistory } from 'react-router';

import { ScheduleChartView1 } from './GanttChart/StyledGanttChart';
import { LoadingAntd } from '../../../components/Loader/Loader';
import { ErrorComponent } from '../../../components/Logic/ErrorComponent';
import { adverContext } from './AdvertisingParties';
import mapData from './utils/mapData';
import getGanttSettings from './GanttChart/ganttSettings';

let globalWidthsGridAndChart = ['20%', '80%'];

const setWidthsGridAndChart = (val) => {
  globalWidthsGridAndChart = val;
};

export function GanttChartAdvertisingSides({ setGanttUpdater, setItemsLength, offset, setPageLoading }) {
  const history = useHistory();
  const {
    chartItems,
    setChartItems,
    period,
    infoColumns,
    changedItems,
    setChangedItems,
    searchQuery,
    orderBy,
    setOrderBy,
  } = useContext(adverContext);

  const ganttSettings = getGanttSettings(period, globalWidthsGridAndChart, setWidthsGridAndChart, orderBy, setOrderBy);

  let loading = searchQuery.loading;
  let error = searchQuery.error;
  let data = searchQuery.data;
  const fetchMore = searchQuery.fetchMore;

  // Эффект поведения пагинации
  useEffect(() => {
    if (data) {
      setPageLoading(true);
      fetchMore({
        variables: {
          offset: offset.offset,
          limit: offset.limit,
        },
      }).then((res) => {
        setChartItems(mapData(res.data.searchAdvertisingSidesOptim.content, history));
        setPageLoading(false);
      });
    }
  }, [offset, data, fetchMore, history, setChartItems, setPageLoading]);

  // Эффект обновления данных
  useEffect(() => {
    if (data && data.searchAdvertisingSidesOptim) {
      let scheduleChartItems = mapData(data.searchAdvertisingSidesOptim.content, history);
      setItemsLength(data.searchAdvertisingSidesOptim.pageInfo.totalCount);
      setChartItems(scheduleChartItems);
    }
  }, [data, history, setChartItems, setItemsLength]);

  if (loading) {
    return (
      <div
        style={{
          display: 'flex',
          justifyContent: 'center',
        }}
      >
        <LoadingAntd/>
      </div>
    );
  }

  if (error)
    return <ErrorComponent error={error}/>;

  return (
    <>
      <ScheduleChartView1
        items={chartItems}
        settings={ganttSettings}
        columns={infoColumns}
        setGanttUpdater={setGanttUpdater}
        changedItems={changedItems}
        setChangedItems={setChangedItems}
      />
    </>
  );
}
