import React from 'react';
import { Popover } from 'antd';
import { useHistory } from 'react-router';
import freeIcon from '../../../img/sales/free.svg';
import bookedIcon from '../../../img/sales/booked.svg';
import soldIcon from '../../../img/sales/sold.svg';
import Tab from './Tab';
import Tablea from '../../../components/Tablea/Tablea_estimate';

export const PanelBatch = (props) => {
  const columns = [
    {
      title: 'Период',
      dataIndex: 'period',
      width: 150,
    },
    {
      title: 'A1',
      dataIndex: 'A1',
      width: 150,
    },
    {
      title: 'A2',
      dataIndex: 'A2',
      width: 150,
    },
    {
      title: 'A3',
      dataIndex: 'A3',
      width: 150,
    },
    {
      title: 'A4',
      dataIndex: 'A4',
      width: 150,
    },
    {
      title: 'B1',
      dataIndex: 'B1',
      width: 150,
    },
    {
      title: 'B2',
      dataIndex: 'B2',
      width: 150,
    },
    {
      title: 'B3',
      dataIndex: 'B3',
      width: 150,
    },
    {
      title: 'B4',
      dataIndex: 'B4',
      width: 150,
    },
    {
      title: 'D1',
      dataIndex: 'D1',
      width: 150,
    },
    {
      title: 'D2',
      dataIndex: 'D2',
      width: 150,
    },
  ];

  const data = [
    {
      period: '16 - 01 марта',
      A1: (
        <Popover content={<Tab history={useHistory()} title="Проект Coca-cola"></Tab>} placement="bottom">
          <p style={{ fontSize: '10px', position: 'absolute', margin: '5px 10px', color: '#D42D11' }}>Продано</p>
          <strong style={{ color: '#00284C', position: 'absolute', fontSize: '12px', margin: '20px 10px' }}>
            Coca-Cola
          </strong>
          <img alt="" src={soldIcon} />
        </Popover>
      ),
      A2: (
        <>
          <p style={{ fontSize: '10px', position: 'absolute', margin: '5px 10px', color: '#78A90D' }}>Свободно</p>
          <img alt="" src={freeIcon} />
        </>
      ),
      A3: (
        <>
          <p style={{ fontSize: '10px', position: 'absolute', margin: '5px 10px', color: '#78A90D' }}>Свободно</p>
          <img alt="" src={freeIcon} />
        </>
      ),
      A4: (
        <>
          <p style={{ fontSize: '10px', position: 'absolute', margin: '5px 10px', color: '#78A90D' }}>Свободно</p>
          <img alt="" src={freeIcon} />
        </>
      ),
      B1: (
        <Popover content={<Tab history={useHistory()} title="Проект Coca-cola"></Tab>} placement="bottom">
          <p style={{ fontSize: '10px', position: 'absolute', margin: '5px 10px', color: '#D42D11' }}>Продано</p>
          <strong style={{ color: '#00284C', position: 'absolute', fontSize: '12px', margin: '20px 10px' }}>
            Coca-Cola
          </strong>
          <img alt="" src={soldIcon} />
        </Popover>
      ),
      B2: (
        <Popover content={<Tab history={useHistory()} title="Проект Jacobs"></Tab>} placement="bottom">
          <p style={{ fontSize: '10px', position: 'absolute', margin: '5px 10px', color: '#117BD4' }}>Забранировано</p>
          <strong style={{ color: '#00284C', position: 'absolute', fontSize: '12px', margin: '20px 10px' }}>
            Jacobs
          </strong>
          <img alt="" src={bookedIcon} />
        </Popover>
      ),
      B3: (
        <Popover content={<Tab history={useHistory()} title="Проект Coca-cola"></Tab>} placement="bottom">
          <p style={{ fontSize: '10px', position: 'absolute', margin: '5px 10px', color: '#D42D11' }}>Продано</p>
          <strong style={{ color: '#00284C', position: 'absolute', fontSize: '12px', margin: '20px 10px' }}>
            Coca-Cola
          </strong>
          <img alt="" src={soldIcon} />
        </Popover>
      ),
      B4: (
        <Popover content={<Tab history={useHistory()} title="Проект Coca-cola"></Tab>} placement="bottom">
          <p style={{ fontSize: '10px', position: 'absolute', margin: '5px 10px', color: '#D42D11' }}>Продано</p>
          <strong style={{ color: '#00284C', position: 'absolute', fontSize: '12px', margin: '20px 10px' }}>
            Coca-Cola
          </strong>
          <img alt="" src={soldIcon} />
        </Popover>
      ),
    },
    {
      period: '02 - 15 марта',
      A1: (
        <>
          <p style={{ fontSize: '10px', position: 'absolute', margin: '5px 10px', color: '#78A90D' }}>Свободно</p>
          <img alt="" src={freeIcon} />
        </>
      ),
      A2: (
        <Popover
          content={
            <Tab
              history={useHistory()}
              title={
                <>
                  <span>Проект Coca-cola</span>
                  <br />
                  <span>Проект Jacobs</span>
                </>
              }
            ></Tab>
          }
          placement="bottom"
        >
          <p style={{ fontSize: '10px', position: 'absolute', margin: '5px 10px', color: '#D42D11' }}>Продано</p>
          <strong style={{ color: '#00284C', position: 'absolute', fontSize: '12px', margin: '20px 10px' }}>
            Coca-Cola
          </strong>
          <strong
            style={{
              color: 'rgb(63, 63, 209)',
              position: 'absolute',
              fontSize: '12px',
              margin: '20px 10px',
              marginTop: '40px',
            }}
          >
            Jacobs
          </strong>
          <img alt="" src={soldIcon} />
        </Popover>
      ),
      A3: (
        <>
          <p style={{ fontSize: '10px', position: 'absolute', margin: '5px 10px', color: '#78A90D' }}>Свободно</p>
          <img alt="" src={freeIcon} />
        </>
      ),
      A4: (
        <Popover content={<Tab history={useHistory()} title="Проект Jacobs"></Tab>} placement="bottom">
          <p style={{ fontSize: '10px', position: 'absolute', margin: '5px 10px', color: '#117BD4' }}>Забранировано</p>
          <strong style={{ color: '#00284C', position: 'absolute', fontSize: '12px', margin: '20px 10px' }}>
            Jacobs
          </strong>
          <img alt="" src={bookedIcon} />
        </Popover>
      ),
      B1: (
        <Popover content={<Tab history={useHistory()} title="Проект Coca-cola"></Tab>} placement="bottom">
          <p style={{ fontSize: '10px', position: 'absolute', margin: '5px 10px', color: '#D42D11' }}>Продано</p>
          <strong style={{ color: '#00284C', position: 'absolute', fontSize: '12px', margin: '20px 10px' }}>
            Coca-Cola
          </strong>
          <img alt="" src={soldIcon} />
        </Popover>
      ),
      B2: (
        <Popover content={<Tab history={useHistory()} title="Проект Coca-cola"></Tab>} placement="bottom">
          <p style={{ fontSize: '10px', position: 'absolute', margin: '5px 10px', color: '#D42D11' }}>Продано</p>
          <strong style={{ color: '#00284C', position: 'absolute', fontSize: '12px', margin: '20px 10px' }}>
            Coca-Cola
          </strong>
          <img alt="" src={soldIcon} />
        </Popover>
      ),
      B3: (
        <>
          <p style={{ fontSize: '10px', position: 'absolute', margin: '5px 10px', color: '#78A90D' }}>Свободно</p>
          <img alt="" src={freeIcon} />
        </>
      ),
      B4: (
        <Popover content={<Tab history={useHistory()} title="Проект Coca-cola"></Tab>} placement="bottom">
          <p style={{ fontSize: '10px', position: 'absolute', margin: '5px 10px', color: '#D42D11' }}>Продано</p>
          <strong style={{ color: '#00284C', position: 'absolute', fontSize: '12px', margin: '20px 10px' }}>
            Coca-Cola
          </strong>
          <img alt="" src={soldIcon} />
        </Popover>
      ),
    },
    {
      period: '16 - 30 марта',
      A1: (
        <Popover content={<Tab history={useHistory()} title="Проект Coca-cola"></Tab>} placement="bottom">
          <p style={{ fontSize: '10px', position: 'absolute', margin: '5px 10px', color: '#D42D11' }}>Продано</p>
          <strong style={{ color: '#00284C', position: 'absolute', fontSize: '12px', margin: '20px 10px' }}>
            Coca-Cola
          </strong>
          <img alt="" src={soldIcon} />
        </Popover>
      ),
      A2: (
        <>
          <p style={{ fontSize: '10px', position: 'absolute', margin: '5px 10px', color: '#78A90D' }}>Свободно</p>
          <img alt="" src={freeIcon} />
        </>
      ),
      A3: (
        <>
          <p style={{ fontSize: '10px', position: 'absolute', margin: '5px 10px', color: '#78A90D' }}>Свободно</p>
          <img alt="" src={freeIcon} />
        </>
      ),
      A4: (
        <>
          <p style={{ fontSize: '10px', position: 'absolute', margin: '5px 10px', color: '#78A90D' }}>Свободно</p>
          <img alt="" src={freeIcon} />
        </>
      ),
      B1: (
        <Popover content={<Tab history={useHistory()} title="Проект Coca-cola"></Tab>} placement="bottom">
          <p style={{ fontSize: '10px', position: 'absolute', margin: '5px 10px', color: '#D42D11' }}>Продано</p>
          <strong style={{ color: '#00284C', position: 'absolute', fontSize: '12px', margin: '20px 10px' }}>
            Coca-Cola
          </strong>
          <img alt="" src={soldIcon} />
        </Popover>
      ),
      B2: (
        <Popover content={<Tab history={useHistory()} title="Проект Jacobs"></Tab>} placement="bottom">
          <p style={{ fontSize: '10px', position: 'absolute', margin: '5px 10px', color: '#117BD4' }}>Забранировано</p>
          <strong style={{ color: '#00284C', position: 'absolute', fontSize: '12px', margin: '20px 10px' }}>
            Jacobs
          </strong>
          <img alt="" src={bookedIcon} />
        </Popover>
      ),
      B3: (
        <Popover content={<Tab history={useHistory()} title="Проект Coca-cola"></Tab>} placement="bottom">
          <p style={{ fontSize: '10px', position: 'absolute', margin: '5px 10px', color: '#D42D11' }}>Продано</p>
          <strong style={{ color: '#00284C', position: 'absolute', fontSize: '12px', margin: '20px 10px' }}>
            Coca-Cola
          </strong>
          <img alt="" src={soldIcon} />
        </Popover>
      ),
      B4: (
        <Popover content={<Tab history={useHistory()} title="Проект Coca-cola"></Tab>} placement="bottom">
          <p style={{ fontSize: '10px', position: 'absolute', margin: '5px 10px', color: '#D42D11' }}>Продано</p>
          <strong style={{ color: '#00284C', position: 'absolute', fontSize: '12px', margin: '20px 10px' }}>
            Coca-Cola
          </strong>
          <img alt="" src={soldIcon} />
        </Popover>
      ),
    },
  ];

  return (
    <>
      <div className="outdoor-table-bar">
        {/*<Table style={{ width: '100%' }} columns={columns} data={data} />*/}
        <Tablea columns={columns} data={data} />
      </div>

      <style>
        {`.outdoor-table-bar {
            width: 100%;
            margin-left:auto;
          }
         `}
      </style>
    </>
  );
};
