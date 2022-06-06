import styled from 'styled-components';
import { Table } from 'antd';

export const StyledTable = styled(Table)`
  border: 1px solid #d3dff0;
  border-radius: 8px 8px 0px 0px;
  width: 100%;
  margin-bottom: 0;

  .ant-table-body {
    //min-height: 500px !important;
    //max-height: calc(100vh - 21rem) !important;
  }

  th {
    color: #1a1a1a !important;
    font-weight: 600 !important;
    // border-right: 1px solid black;
    // margin: 10px;
    border-bottom: 1px solid #d3dff0 !important;
    background: transparent !important;
  }
  // Легкий шрифт в теле таблицы
  td {
    color: #1a1a1a !important;
    font-weight: 500 !important;
  }
  .ant-table {
    background: none;
  }
  .ant-table-content {
    overflow: auto;
  }
  // .ant-table-row td:not(:first-child),
  // .ant-table-row td:not(:first-child) {
  //   padding: 5px !important;
  // }
  // .ant-table-body tr:nth-child(odd),
  // & .ant-table-tbody tr:nth-child(even) {
  //   background: #f5f7fa !important;
  // }
  .ant-table-thead > tr > th {
    height: 48px;
    padding: 0 16px;
    overflow-wrap: anywhere;
  }

  @media all and (max-width: 1300px) {
    .ant-table-thead > tr > th {
      padding: 0 8px;
    }
  }

  .ant-table-thead .ant-table-cell-scrollbar {
    box-shadow: none;
  }
  & .ant-table-thead > tr > th[colspan]:not([colspan='1']) {
    left: 5px;
  }
  & .ant-table-tbody > tr.ant-table-row:hover > td {
    //background: #d3dff0;
    //background: #FFFFFF;
  }
  // Разделитель главных групп заголовка
  & .ant-table-thead > tr > th:not(:last-child) > span {
    background: #d3dff0;
    width: 1px;
    height: calc(100% - 1.2rem);
    margin-top: 0.6rem;
    display: inline-block;
    top: 0;
    right: 0;
  }
  & .ant-table-thead > tr.colhead-multiline-row th.colhead-first-line-first:after {
    // border: none;
  }
  & .ant-table-thead > tr.colhead-multiline-row > th[colspan]:not([colspan='1']) {
    left: 0px;
  }
  & .ant-table-thead > tr.colhead-multiline-row th.colhead-first-line-middle:after {
    border-right: 1px solid #d3dff0 !important;
    right: 0;
    top: 0;
    widhth: 1px;
    margin-top: 0.6rem;
    height: calc(100% - 0.6rem);
    position: absolute;
    display: block;
    content: '.';
    color: rgba(0, 0, 0, 0);
  }
  .ant-table-thead > tr.colhead-multiline-row th.colhead-multiline-multirow:after {
    border-left: 1px solid #d3dff0 !important;
    left: 0;
    top: 0;
    widhth: 100%;
    margin-top: 0.6rem;
    height: calc(100% - 1.2rem);
    position: absolute;
    display: block;
    content: '.';
    color: rgba(0, 0, 0, 0);
  }
  & .ant-table-thead > tr th.colhead-first-line-last:after {
  }
  & .ant-table-thead > tr.colhead-multiline-row > th > span {
    // display: none;
    right: -1px !important;
  }
  // Полосы фона на четных строках
  & .ant-table-tbody tr:nth-child(odd) td {
    background: #f5f7fa !important;
  }
  svg {
    vertical-align: unset;
  }
  // Интервал между строками в теле
  .ant-table-tbody > tr > td,
  .ant-table tfoot > tr > td {
    min-height: 40px;
    padding: 8px 16px;
  }

  @media all and (max-width: 1300px) {
    .ant-table-tbody > tr > td,
    .ant-table tfoot > tr > td {
      padding: 8px 8px;
    }
  }

  // белые линии в таблице
  .ant-table-tbody > tr > td:not(:last-child):not(:first-child):after {
    border-right: 1px solid #ffffff !important;
    right: 2px;
    top: 0;
    widhth: 1px;
    height: 100%;
    position: absolute;
    display: block;
    content: '.';
    color: rgba(0, 0, 0, 0);
  }
  //.ant-table-body tr td:not(:first-child) {
  //  background: #f5f7fa !important;
  //}
  //.ant-table-body tr td:not(:first-child) {
  //  background: #f5f7fa !important;
  //}
  .ant-table-column-sorters {
    padding: 0;
  }
`;

