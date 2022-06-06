import React from 'react';
import { Collapse } from 'antd';

const SidebarInfoItemSum = ({ item }) => {
  return (
    <Collapse.Panel
      header={
        <div className="sidebar-info-container__panel-sum-header">
          <span>{ item.title && item.title }</span>
          <span>{ item.value && item.value }</span>
        </div>
      }
      key={ item.id && item.id }
      extra={<img src={ item.icon && item.icon } alt="title icon"/>}
      showArrow={false}
      disabled={true}
      className="sidebar-info-container__panel-sum"
    >
    </Collapse.Panel>
  )
}

export default SidebarInfoItemSum;
