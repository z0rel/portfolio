import React, { createContext, useState } from 'react';
import { Breadcrumb, Layout } from 'antd';
import { Link } from 'react-router-dom';
import styled from 'styled-components';
import { HeaderTitleWrapper, HeaderWrapper, LeftBar, StyledButton } from '../../../components/Styles/DesignList/styles';
import { useHistory } from 'react-router';

import FilterBar from './FilterBar';

import breadcrumbs from '../../../img/outdoor_furniture/bx-breadcrumbs.svg';
import { TitleLogo } from '../../../components/Styles/ComponentsStyles';
import { ButtonGroup } from '../../../components/Styles/ButtonStyles';
import { JobTitle } from '../../../components/Styles/StyledBlocks';
import SearchBtn from '../../../components/LeftBar/SearchBtn';
import CreateBtn from '../../../components/LeftBar/CreateBtn';

import { PanelBatch } from './PanelBatch';
import { routes } from '../../../routes';

export const batchContext = createContext();
const { Content, Sider } = Layout;

const BatchPlacement = () => {
  const [collapsed, setCollapsed] = useState(true);
  const [filter, setFilter] = useState({});

  const history = useHistory();
  return (
    <batchContext.Provider value={[filter, setFilter]}>
      <Layout>
        <Layout>
          <StyledSider>
            <StyledSider>
              <LeftBar>
                <SearchBtn onClick={() => setCollapsed(!collapsed)}/>
                <CreateBtn
                  text="Создать проект"
                  onClick={() => {
                    history.push(routes.sales.project_new.path);
                  }}
                />
              </LeftBar>
            </StyledSider>
          </StyledSider>
          {collapsed && <FilterBar/>}
          <StyledLayout>
            <StyledBreadcrumb>
              <Breadcrumb.Item>
                <img alt="" src={breadcrumbs}/>
                <Link to="/">Главная</Link>
              </Breadcrumb.Item>
              <Breadcrumb.Item>
                <Link to="/sales/">Продажи</Link>
              </Breadcrumb.Item>
              <Breadcrumb.Item>Пакетное размещение</Breadcrumb.Item>
            </StyledBreadcrumb>
            <HeaderWrapper>
              <HeaderTitleWrapper>
                <TitleLogo/>
                <JobTitle>Пакетное размещение</JobTitle>
              </HeaderTitleWrapper>
              <ButtonGroup>
                <StyledButton backgroundColor="#FF5800">Создать Отчёт</StyledButton>
              </ButtonGroup>
            </HeaderWrapper>

            <Content>
              <PanelBatch style={{ flex: '0 1 auto' }}/>
            </Content>
          </StyledLayout>
        </Layout>

        <style>{`

        .dot-1 {
          height: 8px;
          margin: 0 4px;
          width: 8px;
          background-color: #63D411;
          border-radius: 50%;
          display: inline-block;
        }
        .dot-2 {
          height: 8px;
          margin: 0 4px;
          width: 8px;
          background-color: #117BD4;
          border-radius: 50%;
          display: inline-block;
        }
        .dot-3 {
          height: 8px;
          margin: 0 4px;
          width: 8px;
          background-color: #FDC911;
          border-radius: 50%;
          display: inline-block;
        }
        .dot-4 {
          height: 8px;
          margin: 0 4px;
          width: 8px;
          background-color: #D42D11;
          border-radius: 50%;
          display: inline-block;
        }



      `}</style>
      </Layout>
    </batchContext.Provider>
  );
};

export default BatchPlacement;

const StyledLayout = styled(Layout)`
  background: #fff !important;
  height: 100% !important;
  padding: 30px 30px 0 30px;
`;

const StyledSider = styled(Sider)`
  background: #f5f7fa;
  min-width: 60px !important;
  max-width: 60px !important;
  border-right: 1px solid #d3dff0 !important;
`;
const StyledBreadcrumb = styled(Breadcrumb)`
  font-size: 11px;
  margin: 0 0 20px 0;

  a,
  span {
    color: #8aa1c1 !important;
  }

  img {
    margin: 0 8px 0 0;
  }
`;
