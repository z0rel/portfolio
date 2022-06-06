import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import { QUERY_BRANDS } from '../../../containers/Installations/Projects/queries';
import { ReactComponent as BrandIcon } from '../../../img/input/suitcase.svg';
import React from 'react';
import { StyledFormItem } from './StyledFormItem';

export const verboseBrandRow = (x) =>
  `${x.title}${x?.workingSector?.description ? ' / ' : ''}${x?.workingSector?.description}${
    x?.workingSector?.description ? ' / ' : ''
  }${x?.workingSector?.title}` || '';

export const DebouncedSelectBrand = ({
  formitem = StyledFormItem,
  name = 'brandId',
  showDescription = false,
  ...props
}) => {
  return (
    <DebouncedSelect
      dropdownAlignBottom
      name={name}
      label="Бренд"
      formitem={{ formitem: formitem }}
      query={QUERY_BRANDS}
      getQueryVariables={(term) => ({ title_Icontains: term })}
      placeholderSpec={{
        svg: BrandIcon,
        title: 'Бренд',
        svgMarginTop: 0,
        needSvgInDropdown: true,
        titleMarginLeft: '-.5rem',
      }}
      valueSelector={(node) => node?.id}
      queryKey="searchBrand"
      dataUnpackSpec={
        showDescription
          ? {
              unpackForLocalCompare: verboseBrandRow,
            }
          : {
              unpackNodeKey: 'title',
            }
      }
      {...props}
    />
  );
};
