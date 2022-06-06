import { Layout, Menu } from 'antd';
import { LeftBar } from '../../../../components/Styles/DesignList/styles';
import SearchBtn from '../../../../components/LeftBar/SearchBtn';
import CreateBtn from '../../../../components/LeftBar/CreateBtn';
import React from 'react';

export const FilterLeftBar = ({setCollapsed, collapsed}) => (
  <Layout.Sider className="layout-sider">
    <Menu
      className="layout-sider"
      mode="inline"
      defaultSelectedKeys={['1']}
      defaultOpenKeys={['sub1']}
      style={{ height: '100%', borderRight: 0 }}>
      <LeftBar>
        <SearchBtn
          onClick={() => {
            setCollapsed(!collapsed);
          }}
        />
        <CreateBtn text="Создать проект" />
      </LeftBar>
    </Menu>
  </Layout.Sider>
);
