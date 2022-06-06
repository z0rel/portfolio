import React, { useState } from 'react';
import { useParams } from 'react-router';
import { useQuery } from '@apollo/client';
import BreadCrumbs from '../../../components/BreadCrumbs/BreadCrumbs';

import { SliderState } from '../../../components/SlidingBottomPanel/SliderState';
import { EditReservationSlider } from './components/BottomSliderEditReservations';

import { getProjectCardSidebarData } from './utils/getProjectCardSidebarData';
import { QUERY_PROJECT_CARD } from './queries/queryProjectCard';
import { ErrorComponent } from '../../../components/Logic/ErrorComponent';
import { getProjectAppendices } from './utils/getProjectAppendices';
import { mapProjectReservations } from './utils/mapProjectReservations';
import { BottomSliderCreateUpdateAppendix } from './components/BottomSliderCreateUpdateAppendix';
import { LoadingAntd } from '../../../components/Loader/Loader';
import {
  StyledPanelProjectCard,
  StyledProjectCardLayout,
  StyledProjectCardWrapper,
  StyledProjectData,
  StyledSidebarInfo,
} from './components/styled';
import { ProjectCardHeader } from './components/ProjectCardHeader';
import { LeftProjectNavBar } from './components/LeftProjectNavBar';
import { QUERY_PROJECT_CARD_RESERVATIONS } from './queries/queryReservations';
import { QUERY_PROJECT_CARD_APPENDICES } from './queries/queryAppendices';
import { arrayConnectionString } from '../../../components/Logic/arrayConnectionString';
import { getProjectCardBreadCrumbLinks } from './components/getProjectCardBreadCrumbLinks';
import { getProjectDataItem } from './utils/getProjectDataItem';
import { useFastSearch } from '../../../components/Tablea/logic/fastSearchHook';

const Project_card = () => {
  const sliderState = new SliderState({ name: '', key: '' });

  const { id } = useParams();
  const [choosedBlock, setChoosedBlock] = useState(0);
  const [selectedItems, setSelectedItems] = useState([]);
  const [showEditReservationSlider, setShowEditReservationSlider] = useState(false);
  const [editItemReservationSlider, setEditItemReservationSlider] = useState(null);

  const [showEditAppendixSlider, setShowEditAppendixSlider] = useState(false);
  const [editItemAppendixSlider, setEditItemAppendixSlider] = useState(null);

  const dataProjectCard = useQuery(QUERY_PROJECT_CARD, { variables: { id: id } });

  const pageInfoState = {
    reservations: {
      pageInfo: useState({ limit: 10, offset: arrayConnectionString(0) }),
      orderBy: useState({ orderBy: '' }),
    },
    appendices: {
      pageInfo: useState({ limit: 10, offset: arrayConnectionString(0) }),
      orderBy: useState({ orderBy: '' }),
    },
  };

  const [fastSearchQuery, onChangeFastSearchValue] = useFastSearch('');

  const dataReservations = useQuery(QUERY_PROJECT_CARD_RESERVATIONS, {
    variables: { projectId: id, ...pageInfoState.reservations.pageInfo[0], ...pageInfoState.reservations.orderBy[0], fastSearch: fastSearchQuery },
  });
  const dataAppendices = useQuery(QUERY_PROJECT_CARD_APPENDICES, {
    variables: { projectId: id, ...pageInfoState.appendices.pageInfo[0], ...pageInfoState.appendices.orderBy[0], fastSearch: fastSearchQuery },
  });

  let loading = dataProjectCard.loading;
  let error = dataProjectCard.error;

  if (error)
    return <ErrorComponent error={error}/>;

  const closeEditAppendixSlider = () => {
    setShowEditAppendixSlider(false);
    setEditItemAppendixSlider(null);
  };
  const openEditAppendixSlider = () => setShowEditAppendixSlider(true);

  const panelData = {
    appendices: getProjectAppendices(dataAppendices, pageInfoState.appendices),
    reservations: mapProjectReservations(dataReservations, pageInfoState.reservations),
  };

  const startEditItemAppendix = (item) => {
    setEditItemAppendixSlider(item);
    setShowEditAppendixSlider(true);
  };

  const startEditItemReservation = (item) => {
    setEditItemReservationSlider(item);
    setShowEditReservationSlider(true);
  }
  const closeEditReservationSlider = () => {
    setShowEditReservationSlider(false);
    setEditItemReservationSlider(null);
  };

  if (loading)
    return <LoadingAntd/>;

  const projectDataItem = getProjectDataItem(dataProjectCard.data);

  const links = getProjectCardBreadCrumbLinks(id, projectDataItem?.code);

  return (
    <StyledProjectCardLayout>
      <LeftProjectNavBar id={id}/>
      <StyledProjectCardWrapper>
        <BreadCrumbs links={links} fromRoot={true}/>
        <ProjectCardHeader dataItem={projectDataItem?.title} id={id} handlerOpenCreateAppSlider={openEditAppendixSlider}/>
        <StyledProjectData>
          <StyledSidebarInfo
            data={getProjectCardSidebarData(projectDataItem)}
            className={'sidebar-info-container-mini'}
          />
          <StyledPanelProjectCard
            choosedBlock={choosedBlock}
            setChoosedBlock={setChoosedBlock}
            panelData={panelData}
            setSelectedItems={setSelectedItems}
            startEditItemAppendix={startEditItemAppendix}
            startEditItemReservation={startEditItemReservation}
            onFastSearchSearch={onChangeFastSearchValue}
          />
        </StyledProjectData>
      </StyledProjectCardWrapper>
      {showEditReservationSlider && (
        <EditReservationSlider
          onClose={closeEditReservationSlider}
          setShowed={setShowEditReservationSlider}
          reservation={editItemReservationSlider}
          refetch={dataReservations.refetch}
          projectId={id}
          selectedItems={selectedItems}
        />
      )}
      {showEditAppendixSlider && (
        <BottomSliderCreateUpdateAppendix
          onClose={closeEditAppendixSlider}
          projectId={id}
          refetch={dataAppendices.refetch}
          editItem={editItemAppendixSlider}
        />
      )}
    </StyledProjectCardLayout>
  );
};

export default Project_card;
