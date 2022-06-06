import styled from 'styled-components';
import MaskedInput from 'antd-mask-input'
import { Select, Input, DatePicker } from 'antd';
import './styles.scss'

export const LeftBar = styled.div`
  border-right: 1px solid #d3dff0;
  background-color: #f5f7fa;
  display: flex;
  align-items: center;
  flex-direction: column;
  min-width: 60px;
  max-width: 60px;
`;

export const StyledButton = styled.button`
  background: ${(props) => props.backgroundColor || 'grey'};
  padding: 13px 25px;
  border-radius: 4px;
  margin: 0 0 0 23px;
  font-family: 'SF UI Display Light', sans-serif;
  white-space: nowrap;
  font-size: 14px;
  line-height: 14px;
  color: #ffffff;
  border: none;
`;

export const HeaderWrapper = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin: 0 0 35px 0;
`;

export const HeaderTitleWrapper = styled.div`
  display: flex;
  justify-content: space-between;
`;

// Input components

export const StyledSelect = styled(Select)`

  display: flex;
  align-items: center;
  & > div {
    height: 40px !important;
    display: flex;
    align-items: center;
  }
  .ant-select-selection-placeholder >span{
    margin-left: 5px;
  }
  .ant-select-selection-item > span {
    margin-left: 5px;
  }

`;

export const StyledInput = styled(Input)`
  .ant-input {
    font-size: 16px;
    font-family: 'SF UI Display Light', sans-serif;
  }
  .ant-input-prefix {
    margin-right: 6px;
  }

  height: 40px;
  color: #656565;

`;

export const StyledMaskedInput = styled(MaskedInput)`
  .ant-input{
    font-size: 14px;
  }

  height: 40px;
  color: #656565;
`;

export const StyledDatePicker = styled(DatePicker)`
  height: 40px;
  width: 100%;
`;

export const Chip = styled.div`
  width: 119px;
  height: 30px;
  padding: 8px;
  display: flex;
  align-items: center;
  margin-right: 5px;
  margin-bottom: 3px;
  background: #EEF3FF;
  border: 1px dashed #2C5DE5;
  border-radius: 4px;
  min-width: 15rem;

  &>img {
    width: 14px;
    height: 14px;
    margin-right: 8px;
    cursor: pointer;
  }
  &>span {
    font-family: 'SF UI Display Light', sans-serif;
    font-size: 12px;
    line-height: 20px;
    color: #1A1A1A;
  }
  &:last-child {
    margin-right: 0;
  }
`;

export const DesignList = styled.div`
  display: grid;
  grid-gap: 30px;
  grid-template-columns: repeat(auto-fill,minmax(200px, 1fr));
`;

export const DesignListItem = styled.div`
  background: #FFFFFF;
  border: 1px solid #E7EEF8;
  box-shadow: 0px 5px 10px rgba(0, 0, 0, 0.04), 0px 7px 18px rgba(0, 0, 0, 0.05);
  border-radius: 8px;

  .design-list__item-b-image {
    padding: 5px;
    border-radius: 8px;
    overflow: hidden;
    position: relative;
    margin-bottom: 11px;

    .design-list__item-image {
      width: 100%;
      height: auto;
    }
    .design-list__item-label {
      position: absolute;
      top: 10px;
      right: 10px;
      background: #D42D11;
      border-radius: 8px;
      padding: 6px 11px;
      font-family: 'SF UI Display Light', sans-serif;
      font-size: 9px;
      line-height: 11px;
      text-align: center;
      text-transform: uppercase;
      color: #FFFFFF;
    }
  }
  .design-list__item-title {
    font-family: 'SF UI Display Light', sans-serif;
    font-size: 15px;
    line-height: 18px;
    font-weight: bold;
    color: #1A1A1A;
    padding: 0 15px 13px 15px;
    border-bottom: 1px solid #D3DFF0;

    &>span {
      display: block;
      width: 60%;
    }
  }
  .design-list__item-footer {
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 10px 15px;

    .design-list__item-btn-group {
       display: flex;
       align-items: center;
    }
    .design-list__item-btn {
      width: 32px;
      height: 32px;
      background: #FFFFFF;
      border: 1px solid #D3DFF0;
      box-sizing: border-box;
      border-radius: 4px;
      display: flex;
      align-items: center;
      justify-content: center;
      margin-right: 10px;
      cursor: pointer;

      &:last-child {
        margin-right: 0;
      }
    }
    .ant-checkbox-wrapper {
      font-family: 'SF UI Display Light', sans-serif;
      font-size: 14px;
      line-height: 16px;
      color: #1A1A1A;

      .ant-checkbox-inner {
        border: 1px solid #C6CDD6;
        border-radius: 4px;
      }
    }

  }

  &.current-design {
    .design-list__item-b-image {
      .design-list__item-label {
        background: #D42D11;
      }
    }
  }
  &.archive-design {
    .design-list__item-b-image {
      .design-list__item-label {
        background: #096343;
      }
    }
  }
`;

export const DropdownBtn1 = styled.div`
  width: 189px;
  display: flex;
  align-items: center;
  justify-content: flex-start;

  .dropdown-btn-1__logo {
    width: 20px;
    height: 20px;
    margin-right: 12px;
  }
  .dropdown-btn-1__title {
    font-family: 'SF UI Display Light', sans-serif;
    font-size: 16px;
    line-height: 19px;
    font-weight: bold;
    color: #1A1A1A;
    margin: 0;
  }
  .dropdown-btn-1__arrow {
    width: 10px;
    height: 7px;
  }
`;

export const CustomTabList = styled.div`

  display: flex;
  justify-content: space-between;
  align-items: center;
`;

export const CustomTabBtn = styled.button`
  border-radius: 4px;
  padding: 10px 12px;
  font-family: 'SF UI Display Light', sans-serif;
  font-size: 12px;
  line-height: 12px;
  background: transparent;
  color: #252525;
  text-transform: uppercase;
  border: 0;

  &.active {
    background: #2C5DE5;
    color: #FFFFFF;
  }
`;
