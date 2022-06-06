// eslint-disable-next-line
import React from 'react';
import * as queries from './queries';
import * as helpers from './helpers';
import { ReactComponent as OwnerIcon } from '../../img/input/owner.svg';

// тип select'a и его имя по умолчанию
// если нужен новый раелизуем его и добавляем сюда
enum SelectTypeName {
  creator = 'creator',
  salesManager = 'salesManager',
  backOfficeManager = 'backOfficeManager',
  brand = 'brand',
  workingSector = 'workingSector',
  client = 'client',
}
type TypeProps = keyof typeof SelectTypeName;

const PLACEHOLDER_SPEC_CONFIG = {
  svg: OwnerIcon,
  svgMarginTop: '.13rem',
  titleMarginLeft: '-.5rem',
  needSvgInDropdown: true,
  svgWidthAttr: '18px',
};

const DEFAULT_ITEM_CONFIG = {
  dropdownAlignBottom: true,
  query: queries.SEARCH_CREATORS,
  getQueryVariables: helpers.managerSelectFilter,
  formitem: { antd: true },
  placeholderSpec: {
    ...PLACEHOLDER_SPEC_CONFIG,
  },
  queryKey: 'searchUser',
  dataUnpackSpec: { unpackForLocalCompare: helpers.getManagerName, unpackNodeFun: () => ('') },
  valueSelector: (node: {id: string} | null): string => String(node?.id),
}

// определим для каждого типа нужные пропсы, в результате мы сможем вызывать
// компонент передавая лишь type и name (при необходимости)
const SELECT_CONFIG: Record<string, unknown> = {
  [SelectTypeName.creator]: {
    ...DEFAULT_ITEM_CONFIG,
    label: 'Создатель',
    placeholderSpec: {
      ...PLACEHOLDER_SPEC_CONFIG,
      title: 'Создатель',
    },
  },
  [SelectTypeName.salesManager]: {
    ...DEFAULT_ITEM_CONFIG,
    label: 'Менеджер по продажам',
    placeholderSpec: {
      ...PLACEHOLDER_SPEC_CONFIG,
      title: 'Менеджер по продажам',
    },
  },
  [SelectTypeName.backOfficeManager]: {
    ...DEFAULT_ITEM_CONFIG,
    label: 'Менеджер бэк-офиса',
    placeholderSpec: {
      ...PLACEHOLDER_SPEC_CONFIG,
      title: 'Менеджер бэк-офиса',
    },
  },
  [SelectTypeName.brand]: {
    ...DEFAULT_ITEM_CONFIG,
    dropdownAlignBottom: false,
    label: 'Бренд',
    placeholderSpec: {
      ...PLACEHOLDER_SPEC_CONFIG,
      needSvgInDropdown: false,
      title: 'Бренд',
    },
    queryKey: 'searchBrand',
    query: queries.SEARCH_BRANDS,
    dataUnpackSpec: { unpackForLocalCompare: helpers.getBrandName, unpackNodeFun: () => ('') },
  },
  [SelectTypeName.workingSector]: {
    ...DEFAULT_ITEM_CONFIG,
    dropdownAlignBottom: false,
    label: 'Сектор деятельности',
    placeholderSpec: {
      ...PLACEHOLDER_SPEC_CONFIG,
      needSvgInDropdown: false,
      title: 'Сектор деятельности',
    },
    queryKey: 'searchWorkingSector',
    query: queries.SEARCH_WORK_SECTOR,
    dataUnpackSpec: { unpackForLocalCompare: helpers.getBrandName, unpackNodeFun: () => ('') },
  },
  [SelectTypeName.client]: {
    ...DEFAULT_ITEM_CONFIG,
    dropdownAlignBottom: false,
    label: 'Рекламодатель',
    placeholderSpec: {
      ...PLACEHOLDER_SPEC_CONFIG,
      needSvgInDropdown: false,
      title: 'Рекламодатель',
    },
    queryKey: 'searchPartner',
    query: queries.SEARCH_ADVERTISER,
    getQueryVariables: helpers.clientSelectFilter,
    dataUnpackSpec: { unpackForLocalCompare: helpers.getBrandName, unpackNodeFun: () => ('') },
  }
}

const getPropsByType = (type: TypeProps): Record<string, unknown> => {
  if (SELECT_CONFIG[type]) {
    return SELECT_CONFIG[type] as Record<string, unknown>;
  }

  throw new Error('Неизвестный "type".');
};

export { getPropsByType }
export type { SelectTypeName }
