import React from 'react';
import MainBase from './containers/Base/Main/Main';
// routes
import OutdoorFurniture from './containers/Base/OutdoorFurniture/OutdoorFurniture';
import Brands from './containers/Base/Brands/Brands';
import Partners from './containers/Base/Partners/Partners';
import Partner from './containers/Base/Partner/Partner';
import Brand from './containers/Base/Brand/Brand';
import Construction from './containers/Base/Construction/Construction';
import Locations from './containers/Base/Locations/Locations';
import Location from './containers/Base/Location/Location';
import Agreements from './containers/Base/Documents/Agreements/Agreements';
import Agreement from './containers/Base/Documents/Agreement/Agreement';
import AppendixBase from './containers/Base/Documents/Appendix/Appendix';
import Crews from './containers/Base/Crews/Crews';
import Summary from './containers/Sales/Summary/Summary';
import Invoice from './containers/Sales/Invoice/Invoice';
import Com_projects from './containers/Sales/Com_projects/Com_projects';
import Estimate from './containers/Sales/Estimate/Estimate';
import Appendix from './containers/Sales/Appendix/Appendix';
import Project_card_S from './containers/Sales/Project_card/Project_card';
import Project_new_S from './containers/Sales/Project_new/Project_new';
import Project_edit_S from './containers/Sales/Project_edit/Project_edit';
import AdvertisingParties from './containers/Sales/AdvertisingParties/AdvertisingParties';
import BatchPlacement from './containers/Sales/BatchPlacement/BatchPlacement';
import MainSales from './containers/Sales/Main/Main';
import Main from './containers/Main/Main';
import Design from './containers/Installations/Design/Design';
import MainInstall from './containers/Installations/Main/Main';
import Projects_I from './containers/Installations/Projects/Projects';
import Project_card_I from './containers/Installations/Project_card/Project_card';
import Orders from './containers/Installations/Orders/Orders';
import MainAdministration from './containers/Administration/Main/Main';
import Person from './containers/Administration/Person/Person';
import AdminOutdoorFurniture from './containers/Administration/AdminFormats/FormatsPanel';
import AdminCitiesPanel from './containers/Administration/AdminLocations/AdminCitiesPanel';
import Packages from './containers/Administration/Packages/Packages';
import Prices from './containers/Administration/Prices/Prices';
import AdminCrews from './containers/Administration/AdminCrews/AdminCrews';

export const routes = {
  root: { root: { path: '/', title: 'Главная', component: Main } },
  sales: {
    root: { path: '/sales', name: '', idx: -1, title: 'Продажи', component: MainSales },
    advertising_parties: {
      path: '/sales/advertising_parties',
      name: 'Справочник рекламных сторон',
      iconName: 'carousel',
      idx: 1,
      component: AdvertisingParties,
    },
    advertising_parties_where_come: {
      path: '/sales/advertising_parties/:where_come?',
      name: 'Справочник рекламных сторон',
      idx: -1,
      component: AdvertisingParties,
      url: (where_come) => `/sales/advertising_parties/${where_come}`,
    },
    batch_placement: { 
      path: '/sales/batch_placement', 
      name: 'Пакетное размещение',
      iconName: 'duplicate',
      idx: 2, 
      component: BatchPlacement },
    project_new: { path: '/sales/project_new', name: 'Создание проекта', idx: -1, component: Project_new_S },
    project_edit: {
      path: '/sales/project_edit/:id?',
      name: 'Редактирование проекта',
      generateName: (code) => `Редактирование проекта ${code}`,
      idx: -1,
      component: Project_edit_S,
      url: (id) => `/sales/project_edit/${id}`,
    },
    project_card: {
      path: '/sales/project_card/:id?',
      name: 'Проект',
      generateName: (code) => `Проект ${code}`,
      idx: -1,
      component: Project_card_S,
      url: (id) => `/sales/project_card/${id}`,
    },
    project_card_advertising_parties: {
      path: '/sales/project_card/:id?/advertising_parties',
      name: 'Проект',
      generateName: (code) => `Справочник рекламных сторон – проект ${code}`,
      idx: -1,
      component: AdvertisingParties,
      url: (id) => `/sales/project_card/${id}/advertising_parties`,
      startPrefix: '/sales/project_card/'
    },
    project_estimate: {
      path: '/sales/project_card/:id?/estimate',
      name: 'Смета проекта',
      idx: -1,
      component: Estimate,
      url: (id) => `/sales/project_card/${id}/estimate`,
    },
    appendix: {
      path: '/sales/appendix/:appId',
      name: 'Приложение',
      idx: -1,
      component: Appendix,
      url: (id) => `/sales/appendix/${id}`,
    },
    appendix_estimate: {
      path: '/sales/appendix/:appId/estimate',
      name: 'Смета приложения',
      idx: -1,
      component: Estimate,
      url: (id) => `/sales/appendix/${id}/estimate`,
    },
    com_projects: { path: '/sales/com_projects', name: 'Коммерческие проекты', iconName: 'briefcase', idx: 3, component: Com_projects },
    invoice: { path: '/sales/invoice', name: 'Выставление счета', iconName: 'file',  idx: 4, component: Invoice },
    summary: {
      path: '/sales/summary/:id',
      name: 'Сводка',
      idx: -1,
      component: Summary,
      url: (id) => `/sales/summary/${id}`,
    },
  },
  bases: {
    root: { path: '/base', name: 'Базы', component: MainBase },
    outdoor_furniture: { path: '/base/outdoor_furniture', name: 'Конструкции', iconName: 'collection', component: OutdoorFurniture, idx: 1 },
    locations: { path: '/base/locations', name: 'Список местоположений', iconName: 'map', component: Locations, idx: 2 },
    brands: { path: '/base/brands', name: 'Бренды', iconName: 'bold', component: Brands, idx: 3 },
    partners: { path: '/base/partners', name: 'Контрагенты', iconName: 'group', component: Partners, idx: 4 },
    agreements: { path: '/base/documents/agreements', name: 'Документы', iconName: 'archive', component: Agreements, idx: 5 },
    crews: { path: '/base/crews', name: 'Экипажи',  iconName: 'user-x', component: Crews, idx: 6 },
    partner: {
      path: '/base/partners/partner/:id?',
      name: 'Контрагент',
      component: Partner,
      url: (id) => `/base/partners/partner/${id}`,
    },
    partner_brands: {
      path: '/base/partners/partner/:id?/brands',
      name: 'Связанные бренды',
      component: Brands,
      url: (id) => `/base/partners/partner/${id}/brands`,
    },
    partner_advertisers: {
      path: '/base/partners/partner/:id?/advertisers',
      name: 'Связанные рекламодатели',
      component: Partners,
      url: (id) => `/base/partners/partner/${id}/advertisers`,
    },
    brand: {
      path: '/base/partner/brand/:id?',
      name: 'Бренд',
      component: Brand,
      url: (id) => `/base/partner/brand/${id}`,
    },
    construction: {
      path: '/base/construction/:id?',
      name: 'Конструкция',
      component: Construction,
      url: (id) => `/base/construction/${id}`,
    },
    location: {
      path: '/base/locations/location/:id?',
      name: 'Местоположение',
      component: Location,
      url: (id) => `/base/locations/location/${id}`,
    },
    construction_add_location: {
      path: '/base/construction/:id?/locations',
      name: 'Добавление местоположения',
      component: Locations,
      url: (id) => `/base/construction/${id}/locations`,
    },
    location_add_construction: {
      path: '/base/locations/location/:id?/add_outdoor_furniture',
      name: 'Добавление конструкции',
      component: OutdoorFurniture,
      url: (id) => `/base/locations/location/${id}/add_outdoor_furniture`,
    },
    agreement: {
      path: '/base/documents/agreement/:id?',
      name: 'Договор',
      component: Agreement,
      url: (id) => `/base/documents/agreement/${id}`,
    },
    appendix: {
      path: '/base/documents/appendix/:id?',
      name: 'Прилолжение',
      component: AppendixBase,
      url: (id) => `/base/documents/appendix/${id}`,
    },
    crew: { path: '/base/crews/:id?', name: 'Экипаж', component: Crews, url: (id) => `/base/crews/${id}` },
    crew_add_construction: {
      path: '/base/crews/:id?/add_outdoor_furniture',
      name: 'Добавить конструкцию экипажу',
      component: () => <OutdoorFurniture isCrew={true}/>,
      url: (id) => `/base/crews/${id}/add_outdoor_furniture`,
    },
  },
  installations: {
    root: { path: '/installations', name: 'Монтажи', title: 'Монтажи', component: MainInstall },
    projects: { path: '/installations/projects', name: 'Подача разнарядки', iconName: 'upload', component: Projects_I, idx: 1 },
    project_card: {
      path: '/installations/project_card/:id',
      name: 'Проект',
      idx: -1,
      component: Project_card_I,
      url: (id, dateMountingStart, dateMountingEnd) => {
        return (dateMountingStart !== undefined && dateMountingEnd !== undefined) ? `/installations/project_card/${id}?dateMountingStart=${dateMountingStart}&dateMountingEnd=${dateMountingEnd}` : `/installations/project_card/${id}`;
      },
    },
    orders: { path: '/installations/orders', name: 'Выгрузка разнарядки', iconName: 'download', component: Orders, idx: 2 },
    design: {
      path: '/installations/:id?/design',
      name: 'Дизайн',
      component: Design,
      url: (id) => `/installations/${id}/design`,
    },
  },
  administration: {
    root: { path: '/administration', name: '', idx: -1, title: 'Администрация', component: MainAdministration },
    person: { path: '/administration/person', name: 'Сотрудники', iconName: 'id-card', idx: 1, component: Person },
    outdoor_furniture: {
      path: '/administration/outdoor_furniture',
      name: 'Конструкции',
      iconName: 'collection',
      idx: 2,
      component: AdminOutdoorFurniture,
    },
    locations: { path: '/administration/locations', name: 'Местоположения', iconName: 'map',  idx: 3, component: AdminCitiesPanel },
    packages: { path: '/administration/packages', name: 'Пакеты', iconName: 'shopping-bag', idx: 4, component: Packages },
    crews: { path: '/administration/crews', name: 'Экипажи', iconName: 'user-x', idx: 5, component: AdminCrews },
    prices: { path: '/administration/prices', name: 'Цены', iconName: 'bar-chart', idx: 6, component: Prices },
  },
};

export function sortRouteByIdx([aKey, aVal], [bKey, bVal]) {
  if ('idx' in aVal && 'idx' in bVal)
    return (aVal.idx < bVal.idx && -1) || (aVal.idx > bVal.idx && 1) || 0;
  else if ('idx' in aVal)
    return -1;
  else if ('idx' in bVal)
    return 1;
  else
    return 0;
}

export const filterRouteShowed = ([key, value]) => 'idx' in value && value.idx > 0;
