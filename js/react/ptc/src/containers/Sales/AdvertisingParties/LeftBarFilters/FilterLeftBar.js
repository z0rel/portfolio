import React from 'react';
import { Layout } from 'antd';
import { LeftBar } from '../../../../components/Styles/DesignList/styles';
import SearchBtn from '../../../../components/LeftBar/SearchBtn';
import CreateBtn from '../../../../components/LeftBar/CreateBtn';
import { useHistory } from 'react-router';
import { routes } from '../../../../routes';

export const FilterLeftBar = ({ setCollapsed, collapsed }) => {
  const history = useHistory();
  return (
    <Layout.Sider className="layout-sider">
      <LeftBar>
        {/* Свернуть или развернуть панель фильтров */}
        <SearchBtn
          onClick={() => {
            setCollapsed(!collapsed);
          }}
        />
        <CreateBtn text="Создать проект" onClick={() => history.push(routes.sales.project_new.path)}/>
      </LeftBar>
    </Layout.Sider>
  );
};
