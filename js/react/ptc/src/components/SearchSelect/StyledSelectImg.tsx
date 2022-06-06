import styled from 'styled-components';
import { Select } from 'antd';

export const StyledSelectImgPlaceholder = styled(Select)`
  display: flex;
  align-items: center;
  font-size: 14px;
  font-weight: 500;
  .ant-select-selector {
    width: 100%;
    height: 100% !important;
    .ant-select-selection-search-input {
      margin-left: 1.4rem !important;
    }
  }
  .ant-select-selection-search > input {
    margin-left: 2rem !important;
  }
  & > div {
    height: 40px !important;
    display: flex;
    align-items: center;
  }
  span.ant-select-selection-placeholder {
    margin-left: 2rem;
    font-size: 16px;
  }
  span.ant-select-selection-item {
    font-size: 16px;
    margin-left: 2rem;
  }
  .ant-select-selection-placeholder {
    margin: 0;
    display: flex;
    .debounced-placeholder-title {
      margin-left: 0rem;
    }
  }
  .ant-select-selection-placeholder {
    font-family: 'SF UI Display Medium', sans-serif;
    overflow: visible;
  }

  .ant-select-arrow {
    position: absolute;
    left: 0;
    top: 0;
    width: 100%;
    .debounced-placeholder-down {
      position: absolute;
      top: 1.4rem;
      right: 0.8rem;
    }
    .debounced-placeholder-prefix {
      position: absolute;
      top: 1rem;
      left: 0.6rem;
    }
  }

  span input.ant-select-selection-search-input {
    margin-left: 1.4rem !important;
  }
  span.ant-select-selection-item {
    margin-left: 1.4rem !important;
  }

  .ant-select-selector > .ant-select-selection-item > svg {
    display: none !important;
  }
`;

export const StyledSelectNoImgPlaceholder = styled(Select)`
  display: flex;
  align-items: center;
  font-size: 14px;
  font-weight: 500;
  .anticon.anticon-search {
      display: none;
  }
  .ant-select-selector {
    width: 100%;
    height: 100% !important;
    .ant-select-selection-search-input {
      margin-left: 0.2rem !important;
    }
    padding: 1px !important;
  }
  .ant-select-selection-search > input {
    margin-left: 2rem !important;
  }
  & > div {
    height: 40px !important;
    display: flex;
    align-items: center;
  }
  span.ant-select-selection-placeholder {
    margin-left: .2rem;
    font-size: 16px;
  }
  span.ant-select-selection-item {
    font-size: 16px;
    margin-left: .2rem !important;
  }
  .ant-select-selection-placeholder {
    margin: 0;
    display: flex;
    .debounced-placeholder-title {
      margin-left: 0rem;
    }
  }
  .ant-select-selection-placeholder {
    font-family: 'SF UI Display Medium', sans-serif;
    overflow: visible;
  }

  .ant-select-arrow {
    position: absolute;
    right: 13px;
    top: 50%;
    width: 10px;
  }

  .ant-select-selector > .ant-select-selection-item > svg {
    display: none !important;
  }
`;
