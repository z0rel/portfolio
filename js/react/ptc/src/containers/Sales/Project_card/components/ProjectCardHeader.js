import { useHistory } from 'react-router';
import { HeaderTitleWrapper, HeaderWrapper, StyledButton } from '../../../../components/Styles/DesignList/styles';
import { TitleLogo } from '../../../../components/Styles/ComponentsStyles';
import { JobTitle } from '../../../../components/Styles/StyledBlocks';
import { ButtonGroup } from '../../../../components/Styles/ButtonStyles';
import { colorAccent, colorAccent2, colorAccent3 } from '../../../../components/Styles/Colors';
import { routes } from '../../../../routes';
import React from 'react';

export const ProjectCardHeader = ({ projectTitle, id, handlerOpenCreateAppSlider }) => {
  const history = useHistory();
  if (projectTitle === 'undefined' || projectTitle === undefined)
    projectTitle = '';

  return (
    <HeaderWrapper>
      <HeaderTitleWrapper>
        <TitleLogo/>
        <JobTitle>{'Проект ' + projectTitle}</JobTitle>
      </HeaderTitleWrapper>
      <ButtonGroup>
        <StyledButton
          backgroundColor={colorAccent2}
          onClick={() => history.push(routes.sales.project_edit.url(id))}
        >
          Редактировать проект
        </StyledButton>
        <StyledButton
          backgroundColor={colorAccent3}
          onClick={() => history.push(routes.sales.summary.url(id))}>
          Формирование сводки проекта
        </StyledButton>
        <StyledButton
          backgroundColor={colorAccent}
          onClick={handlerOpenCreateAppSlider}
        >
          Создать приложение
        </StyledButton>
        <StyledButton
          backgroundColor={colorAccent}
          onClick={() => history.push(routes.sales.project_estimate.url(id))}>
          Смета проекта
        </StyledButton>
      </ButtonGroup>
    </HeaderWrapper>
  );
};
