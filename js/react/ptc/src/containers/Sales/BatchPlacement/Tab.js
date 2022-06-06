import React from 'react';
import { Button, Space } from 'antd';
import { routes } from '../../../routes';
const Tab = (props) => {
  return (
    <Space>
      <span>{props.title}</span>
      <Button
        type="primary"
        style={{ borderRadius: '5px', marginLeft: '5px' }}
        onClick={() => {
          props.history.push(routes.sales.project_card.url(props.id));
        }}>
        Открыть Проект
      </Button>
    </Space>
  );
};
export default Tab;
