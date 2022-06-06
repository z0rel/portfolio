import React from 'react';
import { Collapse } from 'antd';

const SidebarInfoItem = ({ item }) => {
  return (
    <Collapse.Panel
      header={ (item && item.title) || '' }
      key={ (item && item.id) || '' }
      extra={<img src={item.icon && item.icon} alt="title icon"/>}
    >
      <ul className="collapse-content-list">
        {
          item.content && item.content.map((el, index) => (
            <li key={index} className="collapse-content-item">
              <span className="collapse-content-item__title">{ el.title && el.title }</span>
              <span className="collapse-content-item__value">{ el.value && el.value }</span>
            </li>
          ))
        }
      </ul>
    </Collapse.Panel>
  )
}

export default SidebarInfoItem;
