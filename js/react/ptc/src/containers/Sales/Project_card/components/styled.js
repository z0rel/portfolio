import styled from 'styled-components';
import { PanelProjectCard } from '../PanelProject_card';
import SidebarInfo from '../../../../components/SidebarInfo';
import { LeftBar } from '../../../../components/Styles/DesignList/styles';

export const StyledProjectData = styled.div`
  display: flex;
`;

export const StyledProjectCardLayout = styled.div`
  display: flex;
  height: 100%;
`;

export const StyledPanelProjectCard = styled(PanelProjectCard)`
  flex: 0 1 auto;
`;

export const StyledSidebarInfo = styled(SidebarInfo)`
  margin-right: 30px;
`;

export const StyledLeftBar = styled(LeftBar)`
  margin: 0 2vw 0 0;
`;

export const StyledProjectCardWrapper = styled.div`
  width: 100%;
  overflow-x: hidden;
  margin: 0 2vw 0 0;
`;
