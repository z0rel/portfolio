import React, { createContext, useState } from 'react';
import { Checkbox, message } from 'antd';
import { useHistory, useParams } from 'react-router';
import BreadCrumbs from '../../../components/BreadCrumbs/BreadCrumbs';
import SearchBtn from '../../../components/LeftBar/SearchBtn';
import AddBtn from '../../../components/LeftBar/AddBtn';
import styled from 'styled-components';
import EditBtn from '../../../components/LeftBar/EditBtn';
import PaperBtn from '../../../components/LeftBar/PaperBtn';
import CreateBtn from '../../../components/LeftBar/CreateBtn';
import PackageBtn from '../../../components/LeftBar/PackageBtn';
import BoxBtn from '../../../components/LeftBar/BoxBtn';
import { TitleLogo } from '../../../components/Styles/ComponentsStyles';
import { JobTitle } from '../../../components/Styles/StyledBlocks';
import { ButtonGroup } from '../../../components/Styles/ButtonStyles';
import { gql, useQuery } from '@apollo/client';
import { routes } from '../../../routes';

import { ControlToolbar } from '../../../components/Styles/ControlToolbarStyle';
import { HeaderTitleWrapper, HeaderWrapper, LeftBar, StyledButton } from '../../../components/Styles/DesignList/styles';

import PanelEstimate from './PanelEstimate';
import SidebarInfo from '../../../components/SidebarInfo';
import { getSidebarInfoData } from './utils/getSidebarInfoData';
import { LoadingAntd } from '../../../components/Loader/Loader';
import { getAppendixCode, getProjectCodeWithTitle } from '../../../components/Logic/projectCode';
import { colorAccent, colorAccent2 } from '../../../components/Styles/Colors';

export const EstimateContext = createContext();

const QUERY_PROJECT = gql`
  query SearchProjectForEstimate($id: ID) {
    searchProject(id: $id) {
      edges {
        node {
          numInYear
          createdAt
          startDate
          title
        }
      }
    }
  }
`;

const QUERY_APPENDIX = gql`
  query SearchAppendixForEstimate($id: ID) {
    searchAppendix(id: $id) {
      edges {
        node {
          numInMonth
          createdDate
          project {
            id
            startDate
            createdAt
            numInYear
            title
          }
        }
      }
    }
  }
`;

const selectedNone = (selectedRows, block) => {
  let b = selectedRows ? selectedRows[block] : null;
  return !b || (!b.all && (!b.keys || b.keys.length === 0));
};

const Estimate = () => {
  const [sidebarData, setSidebarData] = useState(getSidebarInfoData(null));
  const { appId } = useParams();
  const [choosedBlock, setChoosedBlock] = useState(0);
  const [createModal, setCreateModal] = useState(false);
  const [openEditModal, setOpenEditModal] = useState(false);
  const [openPackageEditModal, setPackageEditModal] = useState(false);
  const [openAddToAppendix, setAddToAppendix] = useState(false);

  const [sort, setSort] = useState('');
  const [periodFilter, setPeriodFilter] = useState('');
  const [selectedRows, setSelectedRows] = useState({});
  const history = useHistory();

  const params = useParams();
  let queryProject = useQuery(QUERY_PROJECT, { variables: { id: params.id } });
  let queryAppendix = useQuery(QUERY_APPENDIX, { variables: { id: params.appId } });
  let { links, title } = getNavbarLinks(queryProject, queryAppendix, params);

  let PackageChangeButton = (
    <>
      <StyledButton
        backgroundColor={colorAccent}
        onClick={() => {
          if (selectedNone(selectedRows, choosedBlock))
            message.error('Для пакетного изменения необходимо выбрать строки таблицы');
          else
            setPackageEditModal(true);
        }}
      >
        Пакетное изменение
      </StyledButton>
      <StyledButton
        backgroundColor={colorAccent2}
        onClick={() => {
          if (selectedNone(selectedRows, choosedBlock))
            message.error('Для добавления в приложение необходимо выбрать строки таблицы');
          else
            setAddToAppendix(true);
        }}
      >
        Добавить в приложение
      </StyledButton>
    </>
  );

  return (
    <EstimateContext.Provider
      value={{
        sort,
        setSort,
        createModal,
        setCreateModal,
        openEditModal,
        setOpenEditModal,
        periodFilter,
        setPeriodFilter,
        setSidebarData,
        openPackageEditModal,
        setPackageEditModal,
        openAddToAppendix,
        setAddToAppendix,
        selectedRows,
        setSelectedRows,
      }}
    >
      <div style={{ display: 'flex', height: '100%' }}>
        <LeftBar className="left-bar">
          <SearchBtn/>
          <CreateBtn text="Добавить бронь" onClick={() => history.push(routes.sales.advertising_parties.path)}/>
          <PackageBtn text="Добавить пакет"/>
          <EditBtn text="Перейти в монтажи" onClick={() => history.push(routes.installations.projects.path)}/>
          <PaperBtn text="Сводка проекта"/>
          <BoxBtn text="Архив дизайнов" onClick={() => history.push(routes.installations.design.path)}/>
          {choosedBlock !== 0 && !appId && <AddBtn text="Добавить расход" onClick={() => setCreateModal(true)}/>}
        </LeftBar>
        <div style={{ width: '100%', overflow: 'hidden', margin: '0 2vw 0 0' }}>
          <BreadCrumbs links={links} fromRoot={true}/>
          <HeaderWrapper>
            <HeaderTitleWrapper>
              <TitleLogo/>
              <JobTitle>{title}</JobTitle>
            </HeaderTitleWrapper>
            <ButtonGroup>
              {choosedBlock !== 0 && !appId && (
                <>
                  <StyledButton backgroundColor={colorAccent2} onClick={() => setCreateModal(true)}>
                    Добавить расход
                  </StyledButton>
                  {PackageChangeButton}
                </>
              )}
              {choosedBlock === 0 && PackageChangeButton}
            </ButtonGroup>
          </HeaderWrapper>
          <div style={{ display: 'flex' }}>
            <InfoWrap>
              <ControlToolbar style={{ fontWeight: '600' }}>
                <span>Посчитать с НДС?</span>
                <Checkbox>Да</Checkbox>
              </ControlToolbar>
              <SidebarInfo data={sidebarData}/>
            </InfoWrap>
            <PanelEstimate block={choosedBlock} setChoosedBlock={setChoosedBlock} isAppendix={params.appId}/>
          </div>
        </div>
        <style>
          {`
          .left-bar {
            margin: 0 2vw 0 0;
          }
          .ant-input-number {
            width: 100%;
          }
          `}
        </style>
      </div>
    </EstimateContext.Provider>
  );
};

export default Estimate;
const InfoWrap = styled.div`
  margin: 0 2vw 0 0;
`;

const getNavbarLinks = (queryProject, queryAppendix, params) => {
  const links = [
    { id: routes.sales.root.path, value: 'Продажи' },
    { id: routes.sales.com_projects.path, value: 'Коммерческие проекты' },
  ];

  let title = '';
  let projectCardLink = null;
  let appendixCardLink = null;
  let estimateLink = { id: '', value: 'Смета' };
  if (params.id && queryProject.data) {
    projectCardLink = { id: routes.sales.project_card.url(params.id), value: 'Проект ' };
    estimateLink.id = routes.sales.project_estimate.url(params.id);
    if (!queryProject.error) {
      if (queryProject.loading)
        title = <LoadingAntd/>;
      else if (queryProject.data?.searchProject?.edges?.length) {
        let d = queryProject.data?.searchProject?.edges[0].node;
        let projectName = getProjectCodeWithTitle(d.createdAt, d.numInYear, d.title);
        title = `Смета проекта ${projectName}`;
        projectCardLink.value += projectName;
      }
      else
        title = 'undefined';
    }
    else
      title = 'API Error';
  }
  else if (params.appId && queryAppendix.data) {
    appendixCardLink = { id: routes.sales.appendix.url(params.appId), value: 'Приложение ' };
    estimateLink.id = routes.sales.appendix_estimate.url(params.id);
    if (!queryAppendix.error) {
      if (queryAppendix.loading)
        title = <LoadingAntd/>;
      else if (queryAppendix.data?.searchAppendix?.edges?.length) {
        let d = queryAppendix.data?.searchAppendix?.edges[0].node;
        let projectName = getProjectCodeWithTitle(d?.project?.createdAt, d?.project?.numInYear, d?.project?.title);
        projectCardLink = { id: routes.sales.project_card.url(d?.project?.id), value: `Проект ${projectName}` };
        let estimateName = getAppendixCode(d.createdDate, d.numInMonth);
        title = `Смета приложения ${estimateName}`;
        appendixCardLink.value += estimateName;
      }
      else
        title = 'undefined';
    }
    else
      title = 'API Error';
  }
  if (projectCardLink)
    links.push(projectCardLink);
  if (appendixCardLink)
    links.push(appendixCardLink);
  if (estimateLink)
    links.push(estimateLink);

  return { links, title };
};
