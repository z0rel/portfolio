import React, { useState } from 'react';
import ReactToPrint from 'react-to-print';
import { Button, Checkbox, Dropdown, Input, Menu } from 'antd';

import searchInputIcon from '../../img/header-bar/search-icon.svg';
import printerIcon from '../../img/header-bar/printer.svg';
import exportIcon from '../../img/header-bar/export.svg';
import settingsIcon from '../../img/header-bar/settings.svg';

import './style.css';

export const changeColumns = (dataIndex, columnsForPopup, setColumnsForPopup) => {
  let localColumnsForPopup = columnsForPopup.map((col) => {
    if (col.children) {
      col.children = col.children.map((item) => {
        if (item.dataIndex && item.dataIndex === dataIndex) {
          item.isShowed = !item.isShowed;
        }
        return item;
      });
    }
    if (col.dataIndex && col.dataIndex === dataIndex) {
      col.isShowed = !col.isShowed;
    }
    return col;
  });
  setColumnsForPopup(localColumnsForPopup);
};

export const menuItemLocal = (data, columnsForPopup, setColumnsForPopup) => (
  <Menu.Item key={data.dataIndex}>
    <Checkbox
      checked={data.isShowed}
      onClick={() => changeColumns(data.dataIndex, columnsForPopup, setColumnsForPopup)}
    >
      {data.title || data.header}
    </Checkbox>
  </Menu.Item>
);

const HeaderBar = ({ children, enableEditQuantityOfColumns, columnsConfig, printData }) => {
  let [dropDownVisible, setDropDownVisible] = useState(false);
  let columnsListPopup = <Menu></Menu>;
  if (enableEditQuantityOfColumns && columnsConfig && columnsConfig.columnsForPopup) {
    columnsListPopup = (
      <Menu>
        {columnsConfig.columnsForPopup.map((col, index) => {
          if (col.children) {
            return (
              <Menu.ItemGroup key={index} title={col.title}>
                {col.children.map((item) =>
                  menuItemLocal(item, columnsConfig.columnsForPopup, columnsConfig.setColumnsForPopup),
                )}
              </Menu.ItemGroup>
            );
          }
          return menuItemLocal(col, columnsConfig.columnsForPopup, columnsConfig.setColumnsForPopup);
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
          <Dropdown
            overlay={columnsListPopup}
            className="header-btn"
            trigger={['click']}
            placement="bottomRight"
            visible={dropDownVisible}
            onVisibleChange={() => setDropDownVisible(!dropDownVisible)}
          >
            <Button style={{ marginLeft: '5px' }} className="header-btn">
              <img src={settingsIcon} alt={'settings icon'}/>
            </Button>
          </Dropdown>
        )}
      </div>
    </div>
  );
};

export default HeaderBar;
