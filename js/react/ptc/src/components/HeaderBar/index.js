import React from 'react';
import ReactToPrint from 'react-to-print';
import { Button, Checkbox, Dropdown, Input, Menu } from 'antd';

import searchInputIcon from '../../img/header-bar/search-icon.svg';
import printerIcon from '../../img/header-bar/printer.svg';
import exportIcon from '../../img/header-bar/export.svg';
import settingsIcon from '../../img/header-bar/settings.svg';

import './style.css';

import { changeColumns } from './utils';

let columnsListPopup = (
  <Menu>
    <Menu.Item>
      <Checkbox>1 menu item</Checkbox>
    </Menu.Item>
    <Menu.Item>
      <Checkbox>2 menu item</Checkbox>
    </Menu.Item>
    <Menu.Divider/>
    <Menu.ItemGroup title="АРЕНДА">
      <Menu.Item>
        <Checkbox>3 menu item</Checkbox>
      </Menu.Item>
      <Menu.Item>
        <Checkbox>4 menu item</Checkbox>
      </Menu.Item>
    </Menu.ItemGroup>
    <Menu.Divider/>
    <Menu.ItemGroup title="НАЛОГ">
      <Menu.Item>
        <Checkbox>5 menu item</Checkbox>
      </Menu.Item>
      <Menu.Item>
        <Checkbox>6 menu item</Checkbox>
      </Menu.Item>
    </Menu.ItemGroup>
  </Menu>
);

const HeaderBar = ({ children, enableEditQuantityOfColumns, columnsConfig, printData }) => {
  if (enableEditQuantityOfColumns && columnsConfig && columnsConfig.columnsForPopup) {
    const menuItemLocal = (data) => (
      <Menu.Item key={data.dataIndex}>
        <Checkbox
          checked={data.isShowed}
          onClick={() =>
            changeColumns(
              data.dataIndex,
              columnsConfig.columnsForPopup,
              columnsConfig.setColumnsForPopup,
              columnsConfig.setColumnsTable,
            )
          }
        >
          {data.title}
        </Checkbox>
      </Menu.Item>
    );

    columnsListPopup = (
      <Menu>
        {columnsConfig.columnsForPopup.map((col, index) => {
          if (col.children) {
            return (
              <Menu.ItemGroup key={index} title={col.title}>
                {col.children.map((item) => menuItemLocal(item))}
              </Menu.ItemGroup>
            );
          }
          return menuItemLocal(col);
        })}
      </Menu>
    );
  }

  return (
    <div className="header-bar">
      <div>{children}</div>
      <div>
        <Input
          style={{ marginLeft: '20px' }}
          placeholder="Быстрый поиск"
          suffix="Найти"
          prefix={<img src={searchInputIcon} alt={'Найти'}/>}
        />
        {printData && <div style={{ display: 'none' }}>{printData.element}</div>}
        <ReactToPrint
          trigger={() => (
            <Button style={{ marginLeft: '5px' }} className="header-btn" disabled={!printData}>
              <img src={printerIcon} alt={'Печать'}/>
            </Button>
          )}
          content={() => {
            return printData && printData.refData.current['printListBlocks'];
          }}
        />
        <Button style={{ width: '180px', display: 'flex', justifyContent: 'space-between' }} className="header-btn">
          <img src={exportIcon} alt={'Экспорт'}/>
          <span>Экспорт</span>
        </Button>

        {enableEditQuantityOfColumns && (
          <Dropdown overlay={columnsListPopup} className="header-btn" trigger={['click']} placement="bottomRight">
            <Button style={{ marginLeft: '5px' }} className="header-btn">
              <img src={settingsIcon} alt={''}/>
            </Button>
          </Dropdown>
        )}
      </div>
    </div>
  );
};

export default HeaderBar;
