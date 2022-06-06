import React from 'react';
import { Collapse } from 'antd';

import arrowDown from '../../img/icon_dropdown_select.svg';

import SidebarInfoItem from './SidebarInfoItem';
import SidebarInfoItemSum from './SidebarInfoItemSum';

import './style.css';

const SidebarInfo = ({ data = [], className=undefined, ...props}) => {
  return (
    <div className={`sidebar-info-container${className ? ' ' + className : ''}`} {...props}>
      <Collapse
        defaultActiveKey={data && data.map(item => item.id)}
        activeKey={data && data.map(item => item.id)}
        expandIcon={
          ({ isActive }) =>
            <img
              src={arrowDown}
              style={{ transform: `rotate(${isActive ? 180 : 0}deg)` }}
              alt="arrow"
            />
        }
        expandIconPosition="right"
          >
            {
              data && data.map(item => (
                item.sumBlock
                ? (SidebarInfoItemSum({ item }))
                : (SidebarInfoItem({ item }))
              ))
            }
      </Collapse>
    </div>
  )
}

export default SidebarInfo;
