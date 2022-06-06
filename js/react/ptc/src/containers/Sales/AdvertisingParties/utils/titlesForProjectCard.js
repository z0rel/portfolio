import { useQuery } from '@apollo/client';
import { QUERY_PROJECT_CARD } from '../../Project_card/queries/queryProjectCard';
import { JobTitle } from '../../../../components/Styles/StyledBlocks';
import React from 'react';
import { routes } from '../../../../routes';
import { ErrorComponent } from '../../../../components/Logic/ErrorComponent';
import { getProjectCardBreadCrumbLinks } from '../../Project_card/components/getProjectCardBreadCrumbLinks';
import BreadCrumbs from '../../../../components/BreadCrumbs/BreadCrumbs';
import { getProjectDataItem } from '../../Project_card/utils/getProjectDataItem';

const getProjectTitle = (data) => {
  const dataItem = getProjectDataItem(data);
  let projectTitle = dataItem?.title;
  let projectCode = dataItem?.code;
  if (projectTitle === 'undefined' || projectTitle === undefined)
    projectTitle = '';
  if (projectCode === 'undefined' || projectCode === undefined)
    projectCode = '';
  else
    projectCode = `${projectCode} `;

  return `${projectCode}${projectTitle}`;
}

export const AdvertisingPartiesProjectCardTitle = ({ id }) => {
  const { loading, error, data } = useQuery(QUERY_PROJECT_CARD, { variables: { id: id } });

  if (loading)
    return <JobTitle>{routes.sales.project_card_advertising_parties.generateName('...')}</JobTitle>;

  if (error)
    return <ErrorComponent error={error}/>;

  return (
    <JobTitle>{routes.sales.project_card_advertising_parties.generateName(getProjectTitle(data))}</JobTitle>
  );
};

export const AdvertisingPartiesProjectCardBreadCrumbs = ({ id }) => {
  const { loading, error, data } = useQuery(QUERY_PROJECT_CARD, { variables: { id: id } });

  if (loading) {
    const links = getProjectCardBreadCrumbLinks(id, '...');
    links.push({
      id: routes.sales.project_card_advertising_parties.url(id),
      value: routes.sales.project_card_advertising_parties.generateName('...'),
    });
    return <BreadCrumbs links={links} fromRoot={true} />;
  }

  if (error)
    return <ErrorComponent error={error}/>;

  const links = getProjectCardBreadCrumbLinks(id, getProjectDataItem(data)?.code);
  links.push({
    id: routes.sales.project_card_advertising_parties.url(id),
    value: routes.sales.project_card_advertising_parties.generateName(getProjectTitle(data)),
  });

  return <BreadCrumbs links={links} fromRoot={true} />;
};
