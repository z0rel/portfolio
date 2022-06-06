import React, { createContext, useState } from 'react';

import { HeaderTitleWrapper, HeaderWrapper, LeftBar, StyledButton } from '../../../components/Styles/DesignList/styles';
import PanelDesign from './PanelCom_projects';
import { BreadCrumbsRoutes } from '../../../components/BreadCrumbs/BreadCrumbs';
import { TitleLogo } from '../../../components/Styles/ComponentsStyles';
import { JobTitle } from '../../../components/Styles/StyledBlocks';
import { ButtonGroup } from '../../../components/Styles/ButtonStyles';
import SearchBtn from '../../../components/LeftBar/SearchBtn';
import CreateBtn from '../../../components/LeftBar/CreateBtn';
import FilterBar from './FilterBar';
import { useHistory } from 'react-router';
import { routes } from '../../../routes';

export const comProjectContext = createContext();

const Com_projects = () => {
  const [collapsed, setCollapsed] = useState(true);
  const [constructionsIdSet, setConstructionsIdSet] = useState([]);
  const [filter, setFilter] = useState({});

  const history = useHistory();

  return (
    <comProjectContext.Provider value={[filter, setFilter, constructionsIdSet, setConstructionsIdSet]}>
      <div style={{ display: 'flex', height: '100%' }}>
        <div
          className="flex-margin"
          style={{
            minHeight: '100vh',
          }}
        >
          <LeftBar>
            <SearchBtn onClick={() => setCollapsed(!collapsed)}/>
            <CreateBtn
              text="Создать проект"
              onClick={() => {
                history.push(routes.sales.project_new.path);
              }}
            />
            <CreateBtn
              text="Добавить бронь"
              onClick={() => {
                history.push(routes.sales.advertising_parties.path);
              }}
            />
          </LeftBar>
          {collapsed && <FilterBar/>}
        </div>

        <div style={{ overflowX: 'hidden', margin: '0 2vw 0 0' }}>
          <BreadCrumbsRoutes links={[routes.root.root, routes.sales.root, routes.sales.com_projects]}/>
          <HeaderWrapper>
            <HeaderTitleWrapper>
              <TitleLogo/>
              <JobTitle>Коммерчeские Проекты</JobTitle>
            </HeaderTitleWrapper>
            <ButtonGroup>
              <StyledButton
                backgroundColor="#2C5DE5"
                onClick={() => {
                  history.push(routes.sales.project_new.path);
                }}
              >
                Создать Проект
              </StyledButton>
              <StyledButton backgroundColor="#FF5800">Создать отчет</StyledButton>
            </ButtonGroup>
          </HeaderWrapper>
          <div style={{ display: 'flex' }}>
            <PanelDesign style={{ flex: '0 1 auto' }}/>
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
    </comProjectContext.Provider>
  );
};

export default Com_projects;
