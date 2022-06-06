import React, { useEffect, useRef } from 'react';
import { Form, message } from 'antd';
import styled from 'styled-components';
import { useParams } from 'react-router-dom';
import { HeaderTitleWrapper, HeaderWrapper, LeftBar } from '../../../components/Styles/DesignList/styles';

import { BreadCrumbsRoutes } from '../../../components/BreadCrumbs/BreadCrumbs';
import { TitleLogo } from '../../../components/Styles/ComponentsStyles';
import { JobTitle } from '../../../components/Styles/StyledBlocks';
import SearchBtn from '../../../components/LeftBar/SearchBtn';
import EditBtn from '../../../components/LeftBar/EditBtn';
import PaperBtn from '../../../components/LeftBar/PaperBtn';
import PackageBtn from '../../../components/LeftBar/PackageBtn';
import BoxBtn from '../../../components/LeftBar/BoxBtn';
import CreateBtn from '../../../components/LeftBar/CreateBtn';
import { QUERY_SEARCH_PROJECT, UPDATE_PROJECT } from './utils/queries';

import { BtnSuccess } from '../../../components/Styles/ButtonStyles';
import { useMutation, useQuery } from '@apollo/client';
import { routes } from '../../../routes';
import { QUERY_PROJECT_CARD as QUERY_PROJECT_CARD_S } from '../Project_card/queries/queryProjectCard';
import { QUERY_PROJECT_CARD as QUERY_PROJECT_CARD_M } from '../../Installations/Project_card/queries';
import { evictProjectsTables } from '../../../components/Logic/evictProjectTables';
import { LoadingAntd } from '../../../components/Loader/Loader';
import { ErrorComponent } from '../../../components/Logic/ErrorComponent';
import { FormItem } from '../../../components/Form/FormItem';
import { ProjectEditForm } from './utils/ProjectEditForm';
import { verboseBrandRow } from '../../../components/CustomDebouncedSelect/selects/DebouncedSelectBrand';
import { mapProjectEditFormToMutation } from './utils/mapProjectEditFormToMutation';

export const StyledFormItemAgency = styled(FormItem)`
  .ant-form-item-label {
    label {
      margin-bottom: 0.5rem;
    }
  }
  margin-bottom: 0.6rem;
`;

const unpackProjectData = (data) => (data && data.searchProject?.edges.length ? data.searchProject?.edges[0] : null);

const preloadedUser = (user) => (user ? [{ key: user.id, title: `${user?.firstName} ${user?.lastName}` }] : undefined);
const preloadedBrand = (brand) => (brand ? [{ key: brand.id, title: verboseBrandRow(brand) }] : undefined);
const preloadedPartner = (partner) => (partner ? [{ key: partner.id, title: partner.title }] : undefined);

const Project_card = () => {
  const { id } = useParams();

  const [form] = Form.useForm();
  const { data, loading, error } = useQuery(QUERY_SEARCH_PROJECT, {
    variables: {
      id,
    },
  });

  const [updateProject] = useMutation(UPDATE_PROJECT, {
    update(cache) {
      evictProjectsTables(cache);
    },
    refetchQueries(mutationResult) {
      let projectId = mutationResult?.data?.updateProject?.project?.id;
      let result = [];
      if (projectId) {
        result = [
          { query: QUERY_PROJECT_CARD_S, variables: { id: projectId } },
          { query: QUERY_PROJECT_CARD_M, variables: { projectId: projectId } },
        ];
      }
      return result;
    },
  });

  // Here's how we'll keep track of our component's mounted state
  const componentIsMounted = useRef(true);
  // Using an empty dependency array ensures this only runs on unmount
  useEffect(() => {
    return () => {
      componentIsMounted.current = false;
    };
  }, []);

  useEffect(() => {
    let unpackedProjectData = unpackProjectData(data);
    if (unpackedProjectData) {
      let project = unpackedProjectData.node;
      form.setFieldsValue({
        title: project?.title,
        creator: project?.creator?.id,
        backOfficeManager: project?.backOfficeManager?.id,
        salesManager: project?.salesManager?.id,
        brandId: project?.brand?.id,
        client: project?.client?.id,
        agency: project?.agency?.id,
        comment: project?.comment,
        agPercent: project?.agencyCommission?.percent,
        agSumm: project?.agencyCommission?.value,
        toRent: project?.agencyCommission?.toRent,
        toNalog: project?.agencyCommission?.toNalog,
        toPrint: project?.agencyCommission?.toPrint,
        toMount: project?.agencyCommission?.toMount,
        toAdditional: project?.agencyCommission?.toAdditional,
        toNonrts: project?.agencyCommission?.toNonrts,
        akContragent: project?.agencyCommission?.agent?.id,
      });
    }
  }, [data, form]);

  if (loading) {
    return <LoadingAntd />;
  }
  if (error) {
    return <ErrorComponent error={error} />;
  }

  let unpackedProjectDataRender = unpackProjectData(data);

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
          links={[
            routes.root.root,
            routes.sales.root,
            routes.sales.com_projects,
            routes.sales.project_card,
            routes.sales.project_edit,
          ]}
          keys={{
            [routes.sales.project_card.name]: {
              titleArg: `${unpackedProjectDataRender?.node?.code || ''} ${
                unpackedProjectDataRender?.node?.title || ''
              }`,
              urlArg: id,
            },
            [routes.sales.project_edit.name]: {
              nopath: true,
            },
          }}
        />
        <HeaderWrapper>
          <HeaderTitleWrapper>
            <TitleLogo />
            <JobTitle>
              Проект {unpackedProjectDataRender?.node?.code} {unpackedProjectDataRender?.node?.title}
            </JobTitle>
          </HeaderTitleWrapper>
          <BtnSuccess
            type="primary"
            size="large"
            onClick={() => {
              updateProject({
                variables: {
                  input: mapProjectEditFormToMutation(form),
                  id,
                },
              })
                .then(() => {
                  message.success('Успешно сохранено!');
                })
                .catch((err) => {
                  message.error(err.toString());
                });
            }}
          >
            Сохранить
          </BtnSuccess>
        </HeaderWrapper>
        <ProjectEditForm
          form={form}
          preloadedBackOfficeManager={preloadedUser(unpackedProjectDataRender?.node?.backOfficeManager)}
          preloadedSalesManager={preloadedUser(unpackedProjectDataRender?.node?.salesManager)}
          preloadedCreator={preloadedUser(unpackedProjectDataRender?.node?.creator)}
          preloadedBrand={preloadedBrand(unpackedProjectDataRender?.node?.brand)}
          preloadedClient={preloadedPartner(unpackedProjectDataRender?.node?.client)}
          preloadedAgency={preloadedPartner(unpackedProjectDataRender?.node?.agency)}
          preloadedAkAgent={preloadedPartner(unpackedProjectDataRender?.node?.agencyCommission?.agent)}
          componentIsMounted={componentIsMounted}
        />
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
