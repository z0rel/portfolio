import * as React from 'react';
import { ReactNode } from 'react';
import styled from 'styled-components';
import { LoadingAntd } from '../../Loader/Loader';
import {
  DataPredicateType,
  DataUnpackSpec,
  DataUnpackType,
  defaultValueSelector,
  getValueSelector,
  NodeToTitleFunctionType,
  PlaceholderSpec,
  QueryKeyType,
  RelayLengthInterface,
  ValueSelectorFunctionType,
  PreloadedKeys,
} from './utils';
import { setDataUnpack } from './setDataUnpack';

const StyledSpanInDropdown = styled.span`
  text-align: center;
  top: 0;
  bottom: 0;
  display: block;
  height: 2rem;
`;

interface MapDataToOptionsProps {
  name: string | undefined, // Название элемента формы
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
  isTagMode?: boolean;
  // Предзагруженные опции для отображения заданных данных без ожидания
  preloadedKeys?: Array<PreloadedKeys>;
}

export const mapDataToOptions = ({
                                   name,
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
                                   isTagMode = false,
                                   preloadedKeys = undefined,
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
  const hasData = !loading && data && (dataPredicate as DataPredicateType)(data);
  if (hasData || (preloadedKeys && preloadedKeys.length)) {
    let arr = dataUnpack && hasData ? dataUnpack(data) : [];
    if (emptyrowTitle !== null && emptyrowTitle !== undefined) {
      arr = [{ node: { title: emptyrowTitle, id: 'EMPTY_ROW_KEY' } }, ...arr];
    }
    // Подготовка предзаданных опций
    const arrIds = new Set(arr.map((x) => x.node?.id || ''));
    const arrPrefix: any = [];
    if (preloadedKeys) {
      preloadedKeys.forEach((x) => {
        if (!arrIds.has(x.key))
          arrPrefix.push({ node: { ...x, _itemIsPreloaded: true } });
      });
    }
    if (arrPrefix.length > 0) {
      arr = [...arrPrefix, ...arr];
    }

    // Маппинг массива полученных опций
    return arr.map(
      ({ node }): ReactNode => {
        const key: string = node?._itemIsPreloaded ? (node.key as string) : node.id || 'EMPTY_KEY';

        const value = node?._itemIsPreloaded ? node.key : (valueSelector && valueSelector(node)) || '';
        renderedNodes.set(key, node);

        const title: string = node?._itemIsPreloaded
          ? (node.title as string)
          : (nodeToTitle && nodeToTitle(node)) || '';

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
              <span>{title || ''}</span>
            </Option>
          );
        }
      },
    );
  }
  return [];
};
