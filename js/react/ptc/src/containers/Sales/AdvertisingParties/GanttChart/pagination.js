import React from 'react';
import { Pagination } from 'antd';

const GanttPagination = ({ length, setOffset }) => {
  // const { setChartItems, allChartItems } = useContext(adverContext);
  return (
    <div
      style={{
        margin: '30px auto 110px',
        display: 'flex',
        justifyContent: 'flex-end',
      }}>
      <Pagination
        showSizeChanger
        total={length}
        defaultPageSize={25}
        pageSizeOptions={['10', '25', '100']}
        onChange={(page, size) => {
          setOffset({
            offset: (page-1)*size,
            limit: size
          })
        }}
      />
    </div>
  );
};

export default GanttPagination;
