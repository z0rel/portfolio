import React, { useState } from 'react';
import { useParams, useLocation } from 'react-router';
import { gql, useQuery } from '@apollo/client';
import { LeftBar, StyledButton, HeaderWrapper, HeaderTitleWrapper } from '../../../components/Styles/DesignList/styles';
import PanelDesign from './PanelSummary';
import BreadCrumbs from '../../../components/BreadCrumbs/BreadCrumbs';
import { TitleLogo } from '../../../components/Styles/ComponentsStyles';
import { JobTitle } from '../../../components/Styles/StyledBlocks';
import { ButtonGroup } from '../../../components/Styles/ButtonStyles';
import SearchBtn from '../../../components/LeftBar/SearchBtn';
import FilterBar from './FilterBar';
import EditBtn from '../../../components/LeftBar/EditBtn';
import PaperBtn from '../../../components/LeftBar/PaperBtn';
import PackageBtn from '../../../components/LeftBar/PackageBtn';
import BoxBtn from '../../../components/LeftBar/BoxBtn';
import CreateBtn from '../../../components/LeftBar/CreateBtn';
import { routes } from '../../../routes';

const SUMMARY_QUERY = gql`
query ($dateFrom: Date, $dateTo: Date) {
  searchSummary {
    summary(dateStart:$dateFrom, dateEnd: $dateTo) {
      edges {
        node {
          city
        }
      }
    }
  }
}
  `;

const Summary = () => {
  const { id } = useParams();
  const location = useLocation();
  console.log('[location.state]', location.state)

  const { data } = useQuery(SUMMARY_QUERY, {
    variables: {
      // dateFrom: location.state.dateFrom,
      // dateTo: location.state.dateTo
    },
  });
  const [collapsed, setCollapsed] = useState(true);
  const [choosedBlock, setChoosedBlock] = useState(0);

  console.log('[data]', data);
  const links = [
    { id: routes.root.root.path, value: 'Главная' },
    { id: routes.sales.root.path, value: 'Продажи' },
    { id: routes.sales.com_projects.path, value: 'Коммерческие проекты' },
    { id: routes.sales.project_card.url(id), value: 'Проект' },
    { id: routes.sales.summary.url(id), value: 'Сводка' },
  ];

  return (
    <div style={{ display: 'flex', height: '100%' }}>
      <div className="flex-margin">
        <LeftBar>
          <SearchBtn onClick={() => setCollapsed(!collapsed)} />
          <CreateBtn text="Добавить бронь" />
          <PackageBtn text="Добавить пакет" />
          <EditBtn text="Перейти в монтажи" />
          <PaperBtn text="Сводка проекта" />
          <BoxBtn text="Архив дизайнов" />
        </LeftBar>
        {collapsed && <FilterBar />}
      </div>

      <div style={{ overflowX: 'hidden', margin: '0 2vw 0 0' }}>
        <BreadCrumbs links={links} fromRoot={true}/>
        <HeaderWrapper>
          <HeaderTitleWrapper>
            <TitleLogo />
            <JobTitle>Сводка - CocaCola</JobTitle>
          </HeaderTitleWrapper>
          <ButtonGroup>
            {choosedBlock === 0 && (
              <>
                <StyledButton backgroundColor="#2C5DE5">Выгрузка данных</StyledButton>
              </>
            )}
          </ButtonGroup>
        </HeaderWrapper>

        <div style={{ display: 'flex' }}>
          <PanelDesign style={{ flex: '0 1 auto' }} setChoosedBlock={setChoosedBlock} />
        </div>
      </div>
      <style>
        {`
         .flex-margin {
            display: flex;
            margin: 0 2vw 0 0;
          }
          .left-bar {
            margin: 0 2vw 0 0;
          }
        `}
      </style>
    </div>
  );
};

export default Summary;
