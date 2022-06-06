import React from 'react';
import { Link } from 'react-router-dom';
import { Card } from 'antd';
import styled from 'styled-components';
import { routes } from '../../../routes';

const Main = () => {
  return (
    <StyledDiv>
      <StyledCard title="Добро пожаловать в Продажи!" bordered={true} style={{ width: 300 }}>
        <Link to={routes.sales.advertising_parties.path}>{routes.sales.advertising_parties.name}</Link>
        <Link to={routes.sales.batch_placement.path}>{routes.sales.batch_placement.name}</Link>
        <Link to={routes.sales.com_projects.path}>{routes.sales.com_projects.name}</Link>
        <Link to={routes.sales.invoice.path}>Счета</Link>
      </StyledCard>
    </StyledDiv>
  );
};

export default Main;

const StyledDiv = styled.div`
  display: flex;
  justify-content: center;
  align-items: center;
  margin-top: 150px;
`;

const StyledCard = styled(Card)`
  & > div {
    display: flex;
    flex-direction: column;
  }
`;
