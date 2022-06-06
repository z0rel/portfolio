import { routes } from '../../../../routes';


export const getProjectCardBreadCrumbLinks = (id, projectCode) => [
  { id: routes.sales.root.path, value: 'Продажи' },
  { id: routes.sales.com_projects.path, value: 'Коммерческие проекты' },
  { id: routes.sales.project_card.url(id), value: `Проект ${projectCode}` },
];
