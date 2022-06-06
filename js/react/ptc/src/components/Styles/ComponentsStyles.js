import styled from 'styled-components';
import { Popover } from 'antd';
import icon_book from '../../img/outdoor_furniture/bx-book.svg';

//Styles for OutdoorFurniture & Outdoor furniture container

export const TitleLogo = styled.div`
  width: 33px;
  height: 32px;
  background-color: #d42d11;
  border-radius: 4px;
  background-image: url(${icon_book});
  background-repeat: no-repeat;
  background-position: center;
`;
export const StyledButton = styled.button`
  width: 200px;
  height: 40px;
  background: #008556;
  border: none;
  font-size: 14px;
  line-height: 14px;
  color: #ffffff;
  outline: none;
  box-shadow: none;
  border-radius: 4px;
  margin: 0 10px;
  :hover {
    cursor: pointer;
    opacity: 0.7;
  }
`;

export const LeftBarStyled = styled.div`
  font-family: 'SF UI Display Light', sans-serif;
  border-right: 1px solid #d3dff0;
  background-color: #f5f7fa;
  display: flex;
  justify-content: center;
  margin-top: 2px;
  height: 100%;
`;

export const PopupStyled = styled(Popover)`
  font-family: 'SF UI Display Light', sans-serif;
  padding: 0;
  min-width: 200px;
`;
