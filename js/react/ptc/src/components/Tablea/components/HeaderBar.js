import React, { useState } from 'react';
import { Button, Dropdown, Menu, Input } from 'antd';
import { menuItemLocal } from '../../HeaderBar/HeaderBarTablea';
import attachIcon from '../../../img/header-bar/attach.svg';
import plusIcon from '../../../img/header-bar/plus-icon.svg';
import minusIcon from '../../../img/header-bar/minus-icon.svg';
import { CustomTabBtn, CustomTabList } from '../../Styles/DesignList/styles';
import searchInputIcon from '../../../img/header-bar/search-icon.svg';
import printerIcon from '../../../img/header-bar/printer.svg';
import exportIcon from '../../../img/header-bar/export.svg';
import settingsIcon from '../../../img/header-bar/settings.svg';
import styled from 'styled-components';

const { Search } = Input;

const StyledNavigationTabs = styled.div`
  display: inline-flex;
  justify-content: space-between;
`;


export const TableaEstimateHeaderBar = ({
                                          enableChoosePeriod, // Включить выбор периода в верхней полоске
                                          title, // Заголовок внутри верхней полоски
                                          chooseTableBtns, // Кнопки (слева) внутри верхней полоски
                                          choosedBlock, // Выбранный блок
                                          setChoosedBlock, // Установить новый выбранный блок
                                          enableGearButton, // Включить кнопку-шестеренку
                                          disableExportButton, // Выключить кнопку экспорта
                                          columnsForPopup, // Столбцы для отображения и кнопки-шестеренки
                                          setColumnsForPopup, // Установщик новых столбцов для кнопки-шестеренки
                                          onFastSearchSearch, // Обработчик клика по кнопке "Найти"
                                          onExportClick // Обработчик клика по кнопке "Экспорт"
                                        }) => {
  const [dropDownVisible, setDropDownVisible] = useState(false);

  let columnsListPopup = <Menu></Menu>;
  if (enableGearButton && columnsForPopup) {
    columnsListPopup = (
      <Menu>
        {columnsForPopup &&
        columnsForPopup.map((col, index) => {
          if (col.children) {
            return (
              <Menu.ItemGroup key={index} title={col.title}>
                {col.children.map((item) => menuItemLocal(item, columnsForPopup, setColumnsForPopup))}
              </Menu.ItemGroup>
            );
          }
          return menuItemLocal(col, columnsForPopup, setColumnsForPopup);
        })}
      </Menu>
    );
  }
  return (
    <div className="header-bar">
      <StyledNavigationTabs>
        {enableChoosePeriod ? (
          <>
            {title ? (
              <div style={{ display: 'flex', alignItems: 'center', paddingLeft: 12 }}>
                <img src={attachIcon} alt=""/>
                <span style={{ minWidth: 'max-content', fontWeight: '600', marginLeft: '12px', fontSize: '16px' }}>
                  {title}
                </span>
              </div>
            ) : (
              <div>
                <div style={{ display: 'flex', marginRight: '1rem' }}>
                  <Button className="header-btn">
                    <img alt="" src={plusIcon}/>
                  </Button>
                  <Button className="header-btn">
                    <img alt="" src={minusIcon}/>
                  </Button>
                </div>
              </div>
            )}
          </>
        ) : (
          <div />
        )}
        {chooseTableBtns && (
          <CustomTabList>
            {/*Верхняя панель с табами*/}
            {chooseTableBtns.map((item, index) => (
              <CustomTabBtn
                key={index}
                className={choosedBlock === index ? 'active' : 'reservations-rts'}
                onClick={() => setChoosedBlock(index) }
              >
                {item.title}
              </CustomTabBtn>
            ))}
          </CustomTabList>
        )}
      </StyledNavigationTabs>

      <div>
        <Search
          style={{ marginLeft: '20px' }}
          placeholder="Быстрый поиск"
          allowClear
          enterButton={<Button onClick={onFastSearchSearch}>Найти</Button>}
          onSearch={onFastSearchSearch}
          prefix={<img alt="" src={searchInputIcon}/>}
        />
        <Button style={{ marginLeft: '5px' }} className="header-btn">
          <img alt="" src={printerIcon}/>
        </Button>
        {!disableExportButton && <Button style={{ width: '180px', display: 'flex', justifyContent: 'space-between' }} className="header-btn"
                onClick={onExportClick}>
          <img alt="" src={exportIcon}/>
          <span>Экспорт</span>
        </Button>}

        {enableGearButton && (
          <Dropdown
            overlay={columnsListPopup}
            className="header-btn"
            trigger={['click']}
            placement="bottomRight"
            visible={dropDownVisible}
            onVisibleChange={() => setDropDownVisible(!dropDownVisible)}
          >
            <Button style={{ marginLeft: '5px' }} className="header-btn">
              <img alt="" src={settingsIcon}/>
            </Button>
          </Dropdown>
        )}
      </div>
    </div>
  );
};
