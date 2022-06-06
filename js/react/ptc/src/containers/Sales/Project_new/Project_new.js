import React, { useEffect, useRef } from 'react';
import { Form } from 'antd';
import { LeftBar, HeaderWrapper, HeaderTitleWrapper } from '../../../components/Styles/DesignList/styles';
import { BreadCrumbsRoutes } from '../../../components/BreadCrumbs/BreadCrumbs';
import { TitleLogo } from '../../../components/Styles/ComponentsStyles';
import { JobTitle } from '../../../components/Styles/StyledBlocks';
import SearchBtn from '../../../components/LeftBar/SearchBtn';
import EditBtn from '../../../components/LeftBar/EditBtn';
import PaperBtn from '../../../components/LeftBar/PaperBtn';
import PackageBtn from '../../../components/LeftBar/PackageBtn';
import BoxBtn from '../../../components/LeftBar/BoxBtn';
import CreateBtn from '../../../components/LeftBar/CreateBtn';
import { PROJECT_CREATOR } from './utils/queries';
import { SubmitButton } from '../../../components/Styles/ButtonStyles';
import { useHistory } from 'react-router';
import { useMutation } from '@apollo/client';
import { routes } from '../../../routes';
import { evictProjectsTables } from '../../../components/Logic/evictProjectTables';
import { ProjectEditForm } from '../Project_edit/utils/ProjectEditForm';
import { mapProjectEditFormToMutation } from '../Project_edit/utils/mapProjectEditFormToMutation';

const Project_card = () => {
  const history = useHistory();
  const [form] = Form.useForm();
  const [createProjectMutation, { data }] = useMutation(PROJECT_CREATOR, {
    update(cache) {
      evictProjectsTables(cache);
    },
  });
  if (data) {
    history.push(routes.sales.project_card.url(data.createProject.project.id));
  }

  // Here's how we'll keep track of our component's mounted state
  const componentIsMounted = useRef(true);
  // Using an empty dependency array ensures this only runs on unmount
  useEffect(() => {
    return () => {
      componentIsMounted.current = false;
    };
  }, []);

  return (
    <div style={{ display: 'flex', height: '100%' }}>
      <LeftBar className="left-bar">
        <SearchBtn />
        <CreateBtn text="Добавить бронь" />
        <PackageBtn text="Добавить пакет" />
        <EditBtn text="Перейти в монтажи" />
        <PaperBtn text="Сводка проекта" />
        <BoxBtn text="Архив дизайнов" />
      </LeftBar>

      <div style={{ width: '100%', overflowX: 'hidden', margin: '0 2vw 0 0' }}>
        <BreadCrumbsRoutes
          links={[routes.root.root, routes.sales.root, routes.sales.com_projects, routes.sales.project_new]}
        />
        <HeaderWrapper>
          <HeaderTitleWrapper>
            <TitleLogo />
            <JobTitle>Новый проект</JobTitle>
          </HeaderTitleWrapper>
          <SubmitButton
            size="large"
            onClick={() => {
              const input = {
                ...mapProjectEditFormToMutation(form),
                startDate: new Date(),
              };
              createProjectMutation({
                variables: {
                  input,
                },
              });

              if (data) {
                componentIsMounted.current = false;
                history.push(routes.sales.project_card.url(data.createProject.project.id));
              }
            }}
          >
            Создать проект
          </SubmitButton>
        </HeaderWrapper>

        <ProjectEditForm form={form} componentIsMounted={componentIsMounted} />
      </div>

      <style>
        {`
          .left-bar {
            margin: 0 2vw 0 0;
          }
          .ant-form-item {
            margin-bottom: 0;
          }
        `}
      </style>
    </div>
  );
};

export default Project_card;
