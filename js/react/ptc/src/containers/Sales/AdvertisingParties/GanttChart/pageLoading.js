import React from 'react';
import { Spin } from 'antd';

const PageLoading = () => {
  return (
    <div
      style={{
        position: 'absolute',
        top: 0,
        left: 0,
        height: '100%',
        width: '100%',
        zIndex: 99,
      }}
    >
      <div
        style={{
          display: 'flex',
          alignItems: 'center',
          height: '100%',
          backgroundColor: 'rgba(255, 255, 255, 0.568)',
          justifyContent: 'center',
        }}
      >
        <Spin size="large"/>
      </div>
    </div>
  );
};

export default PageLoading;
