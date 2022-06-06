import { useHistory } from 'react-router';
import SearchBtn from '../../../../components/LeftBar/SearchBtn';
import CreateBtn from '../../../../components/LeftBar/CreateBtn';
import { routes } from '../../../../routes';
import PackageBtn from '../../../../components/LeftBar/PackageBtn';
import EditBtn from '../../../../components/LeftBar/EditBtn';
import PaperBtn from '../../../../components/LeftBar/PaperBtn';
import BoxBtn from '../../../../components/LeftBar/BoxBtn';
import React from 'react';
import { StyledLeftBar } from './styled';
import { normalizeProjectId } from '../utils/normalizeProjectId';



export const LeftProjectNavBar = ({id}) => {
  if (id) {
    id = normalizeProjectId(id)
  }
  const history = useHistory();
  return (
    <StyledLeftBar className="left-bar">
      <SearchBtn/>
      <CreateBtn text="Добавить бронь" onClick={() => history.push(routes.sales.project_card_advertising_parties.url(id))}/>
      <PackageBtn text="Добавить пакет" onClick={() => history.push(routes.sales.batch_placement.path)}/>
      <EditBtn text="Перейти в монтажи" onClick={() => history.push(routes.installations.projects.path)}/>
      <PaperBtn text="Сводка проекта"/>
      <BoxBtn text="Архив дизайнов"/>
    </StyledLeftBar>
  );
};
