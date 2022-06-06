import styled from 'styled-components';
import { Form } from 'antd';
import * as React from 'react';
import { ReactNode } from 'react';
import { ReactComponent as CollapseDown } from '../../img/input/collapse-down.svg';
import { ReactComponent as CollapseUp } from '../../img/input/collapse-up.svg';
import { FormItem, RulesInterface } from '../Form/FormItem';
import { BuildInPlacements } from 'rc-trigger/lib/interface';
import { DROPDOWN_BOTTOM_ALIGN, DROPDOWN_TOP_ALIGN } from '../Form/dropdownPlacements';
import { DocumentNode } from '@apollo/client';
import { Rule } from 'rc-field-form/lib/interface';
import { SizeType } from 'antd/es/config-provider/SizeContext';
import { IsMounted } from './DebouncedInput';
import { LabeledValue } from 'antd/lib/select';
import { LoadingAntd } from '../Loader/Loader';

/** Тип селектора значения из RelayNode GraphQL запроса */
export type ValueSelectorFunctionType = (node: Record<string, unknown>) => string;

/** Тип проверки наличия данных в GraphQL ответе */
export type DataPredicateType = (data: Record<string, unknown>) => boolean;

/** Relay интерфейс Node */
interface NodeInterface {
  node: {
    id: string;
    [val: string]: unknown;
  };
}

/** Тип функции, распаковывающей выборку в массив объектов node [ {node: ... }, {node: ... }, ... ] */
export type DataUnpackType = (data: Record<string, unknown>) => Array<NodeInterface>;

/** Тип ключа верхнего уровня GraphQL ответа */
export type QueryKeyType = string;

/** Тип функции селектора заголовка из узла массива, полученного после распаковки из dataUnpack */
export type NodeToTitleFunctionType = (queryNodeItem: Record<string, unknown>) => string;

/** Интерфейс параметров распаковки для сортировки и получения списка values */
export interface DataUnpackSpec {
  // Переопределение функции распаковки объектов в список строк, для сортировки через localCompare
  unpackForLocalCompare?: ValueSelectorFunctionType;
  // Функция, преобразующая объект в сравниваемую через localCompare строку
  unpackNodeFun: (value: Record<string, unknown>) => string;
  // Ключ объекта, используемый для сравнения, используется если не задана функция
  unpackNodeKey?: string;
}

export interface PlaceholderSpec {
  // тег img для подстановки в placeholder
  image?: any;
  // svg-иконка для подстановки в placeholder
  svg?: any;
  // margin-top для svg-иконки
  svgMarginTop?: string;
  // margin-left для текста placeholder после иконки
  titleMarginLeft?: string;
  // текст placeholder после иконки
  title: string;
  // нужно ли отображать svg в выпадающем списке или нет
  needSvgInDropdown?: boolean;
  // ширина иконки
  svgWidthAttr?: bigint | string;
}

export interface FormItemInterface {
  antd?: boolean; // if true - then Form.Item, if False then FormItem
  childsOnly?: boolean; // Передать только потомков
  formitem?: React.ReactElement; // Стилизованный вариант FormItem
}

export type GetQueryVariablesType = (term: string) => Record<string, unknown>;

export interface ValueSpecInterface {
  value: any;
  setValue: (arg: any) => any;
}

interface CustomizedSelectProps {
  // имя элемента формы
  name?: string;
  // Предикат - есть ли в выборке данные или нет
  dataPredicate?: DataPredicateType;
  // Функция, распаковывающая выборку в массив объектов node [ {node: ... }, {node: ... }, ... ]
  dataUnpack?: DataUnpackType;
  // Параметры распаковки данных - по ключу, по функции. Если не задана, у объектов node, получаемых из dataUnpack - должен быть задан аттрибут title и id
  dataUnpackSpec?: DataUnpackSpec;
  // Текст строки с вариантом "не выбрано ничего" из ответной выборки. Если на задан - не появляется
  emptyrowTitle?: string;
  // Параметры задания Placeholder
  placeholderSpec?: PlaceholderSpec;
  getQueryVariables?: GetQueryVariablesType;
  // Размещать выпадающий список сверху
  dropdownAlignTop: boolean;
  // Размещать выпадающий список снизу
  dropdownAlignBottom?: boolean;
  // Функция выбора значения из Relay Node
  valueSelector?: ValueSelectorFunctionType;
  // Контейнер Form.Item в который помещается Select. Может быть пустым фрагментом
  formitem: FormItemInterface;
  rules?: RulesInterface | Rule[];
  label?: string;
  disabled: boolean;
  componentIsMounted?: IsMounted;
  // Функция преобразования Relay Node в заголовок
  nodeToTitle?: NodeToTitleFunctionType;
  // Должен ли компонент иметь иконку ввода или нет
  hasInputIcon?: boolean;
  size?: SizeType;
  style?: React.CSSProperties;
  onClear?: any;
  handleValueChanged?: any;
  // Название GraphQL запроса (ключ верхнего уровня после query)
  queryKey?: QueryKeyType;
  valueSpec?: ValueSpecInterface;
  // Выпадающий список открыт по умолчанию (может быть нужно для отладки)
  defaultOpen?: boolean;
}

export interface DebouncedSelectProps extends CustomizedSelectProps {
  query: DocumentNode;
  refetchOnChangeThisVar?: any;
}

export interface IconedSelectProps extends CustomizedSelectProps {
  setSearchText?: any;
  value?: any;
  setValue?: any;
  loading?: any;
  data?: any;
}

// Получить подстановочный текст на основе спецификации
export const getPlaceholder = (placeholderSpec?: PlaceholderSpec): React.ReactElement | null => {
  let style = undefined;
  if (placeholderSpec && placeholderSpec.titleMarginLeft) {
    style = { marginLeft: placeholderSpec.titleMarginLeft };
  }
  return (
    placeholderSpec ? (
      <span style={style} className="debounced-placeholder-title">
        {placeholderSpec.title}
      </span>
    ) : null
  );
};

// Получить иконку слева и кнопку вниз или вверх справа инпута
export const getSuffix = (selectOpened: boolean, placeholderSpec?: PlaceholderSpec): React.ReactElement | undefined => {
  if (placeholderSpec !== undefined) {
    if (placeholderSpec.image) {
      return (
        <>
          <img src={placeholderSpec.image} alt={placeholderSpec.title} className="debounced-placeholder-prefix"/>
          {!selectOpened ? (
            <CollapseDown className="debounced-placeholder-down"/>
          ) : (
            <CollapseUp className="debounced-placeholder-down"/>
          )}
        </>
      );
    }
    else if (placeholderSpec.svg) {
      let style = undefined;
      if (placeholderSpec.svgMarginTop)
        style = { marginTop: placeholderSpec.svgMarginTop };
      return (
        <>
          <placeholderSpec.svg className="debounced-placeholder-prefix" style={style}/>
          {!selectOpened ? (
            <CollapseDown className="debounced-placeholder-down"/>
          ) : (
            <CollapseUp className="debounced-placeholder-down"/>
          )}
        </>
      );
    }
  }
  return undefined;
};

export const getFormItem = (
  formItemSpec: FormItemInterface | undefined | null,
  defaultFormItem: React.ReactNode = Form.Item,
): React.ReactNode => {
  let FormitemReactNode: React.ReactNode = defaultFormItem;

  if (formItemSpec === null || formItemSpec === undefined)
    return FormitemReactNode;
  else if (formItemSpec.formitem)
    FormitemReactNode = formItemSpec.formitem;
  else if (!formItemSpec.antd)
    FormitemReactNode = FormItem;
  return FormitemReactNode;
};

export const getDropdownAlign = (
  dropdownAlignTop: boolean | undefined,
  dropdownAlignBottom: boolean | undefined,
): BuildInPlacements => {
  return dropdownAlignTop ? DROPDOWN_TOP_ALIGN : DROPDOWN_BOTTOM_ALIGN;
};

export type ValueSelectorFunction = (dataUnpackSpec: DataUnpackSpec) => ValueSelectorFunctionType;

export const defaultValueSelector: ValueSelectorFunction = (
  dataUnpackSpec: DataUnpackSpec,
): ValueSelectorFunctionType =>
  dataUnpackSpec.unpackForLocalCompare
    ? dataUnpackSpec.unpackForLocalCompare
    : (value: Record<string, unknown>): string => value[dataUnpackSpec.unpackNodeKey as string] as string;

export const getValueSelector = (
  valueSelector?: ValueSelectorFunctionType,
  dataUnpackSpec?: DataUnpackSpec,
): ValueSelectorFunctionType => {
  if (valueSelector === undefined || valueSelector === null) {
    return dataUnpackSpec
      ? defaultValueSelector(dataUnpackSpec)
      : (node: Record<string, unknown>): string => node.title as string;
  }
  else
    return valueSelector;
};

// Создать функцию распаковки данных по спецификации dataUnpackSpec
const setDataUnpack = (dataUnpackSpec?: DataUnpackSpec, queryKey?: string) => {
  if (dataUnpackSpec && queryKey) {
    if (dataUnpackSpec.unpackForLocalCompare) {
      const unpackForCompareFn = dataUnpackSpec.unpackForLocalCompare;
      return (data: any) => {
        const arr = [...data[queryKey].edges];
        arr.sort((a, b) => unpackForCompareFn(a).localeCompare(unpackForCompareFn(b)));
        return arr;
      };
    }
    else if (dataUnpackSpec.unpackNodeKey) {
      const fun = dataUnpackSpec.unpackNodeFun;
      const key = dataUnpackSpec.unpackNodeKey;
      return (data: any) => {
        const arr = [...data[queryKey].edges];
        arr.sort((a, b) => {
          const va = fun ? fun(a.node) : a.node[key];
          const vb = fun ? fun(b.node) : b.node[key];
          if (va === null || va === undefined)
            return -1;
          if (vb === null || vb === undefined)
            return 1;
          return va.localeCompare(vb);
        });
        return arr;
      };
    }
  }
  return undefined;
};

interface MapDataToOptionsProps {
  loading: any;
  data: Record<string, unknown>;
  Option: any;
  renderedNodes: any;
  dataPredicate?: DataPredicateType;
  dataUnpack?: DataUnpackType;
  dataUnpackSpec?: DataUnpackSpec;
  queryKey?: QueryKeyType;
  emptyrowTitle?: string;
  valueSelector?: ValueSelectorFunctionType;
  placeholderSpec?: PlaceholderSpec;
  nodeToTitle?: NodeToTitleFunctionType;
  // Режим задания тегов в dropdown
}

interface OptionItem extends LabeledValue {
  datatitle?: string;
  toString?: () => string;
}

export interface RelayLengthInterface {
  edges: {
    length: number;
  };
}

export const mapDataToOptions = ({
                                   loading,
                                   data,
                                   Option,
                                   renderedNodes,
                                   dataPredicate = undefined,
                                   dataUnpack = undefined,
                                   dataUnpackSpec = undefined,
                                   queryKey = undefined,
                                   emptyrowTitle = undefined,
                                   valueSelector = undefined,
                                   placeholderSpec = undefined,
                                   nodeToTitle = undefined,
                                 }: MapDataToOptionsProps): Array<ReactNode> => {
  if (queryKey && dataPredicate === undefined) {
    dataPredicate = (data) =>
      data &&
      (data[queryKey] as RelayLengthInterface)?.edges &&
      ((data[queryKey] as RelayLengthInterface)?.edges.length || -1) > 0;
  }

  dataUnpack = dataUnpack !== undefined ? dataUnpack : setDataUnpack(dataUnpackSpec, queryKey);
  valueSelector = getValueSelector(valueSelector, dataUnpackSpec);

  if (nodeToTitle === undefined) {
    if (dataUnpackSpec)
      nodeToTitle = defaultValueSelector(dataUnpackSpec);
    else
      nodeToTitle = (node) => (node.title ? (node.title as string) : 'Нет названия');
  }
  if (loading) {
    let svgStyle = undefined;
    if (placeholderSpec?.needSvgInDropdown)
      svgStyle = placeholderSpec?.svgWidthAttr ? { minWidth: placeholderSpec?.svgWidthAttr } : undefined;

    return [
      <Option
        value="__LOADER"
        className="styled-for-svg-placeholder dropdown-option-loader"
        key="__LOADER"
        title="title"
      >
        {placeholderSpec?.needSvgInDropdown && (
          <placeholderSpec.svg style={svgStyle} className="dropdown-svg-with-loader"/>
        )}
        <StyledSpanInDropdown>
          <LoadingAntd/>
        </StyledSpanInDropdown>
      </Option>,
    ];
  }
  if (!loading && data && (dataPredicate as DataPredicateType)(data)) {
    let arr = dataUnpack ? dataUnpack(data) : [];
    if (emptyrowTitle !== null && emptyrowTitle !== undefined) {
      arr = [{ node: { title: emptyrowTitle, id: 'EMPTY_ROW_KEY' } }, ...arr];
    }

    return arr.map(
      ({ node }, index): ReactNode => {
        const value = (valueSelector && valueSelector(node)) || '';
        const key: string = node.id || `EMPTY_KEY${value}${index}`;
        renderedNodes.set(key, node);
        const title = (nodeToTitle && nodeToTitle(node)) || '';

        if (!placeholderSpec?.needSvgInDropdown) {
          return (
            <Option key={key} value={value} title={title}>
              {title}
            </Option>
          );
        }
        else {
          const svgStyle = placeholderSpec?.svgWidthAttr ? { minWidth: placeholderSpec?.svgWidthAttr } : undefined;
          return (
            <Option key={key} value={value} title={title} className="styled-for-svg-placeholder">
              <placeholderSpec.svg style={svgStyle}/>
              <span>{title}</span>
            </Option>
          );
        }
      },
    );
  }
  return [];
};

const StyledSpanInDropdown = styled.span`
  text-align: center;
  top: 0;
  bottom: 0;
  display: block;
  height: 2rem;
`;
