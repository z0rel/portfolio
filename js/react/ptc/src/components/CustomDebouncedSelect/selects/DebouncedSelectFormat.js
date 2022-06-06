import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import { QUERY_FORMAT } from '../../../containers/Installations/Orders/queries';
import { ReactComponent as AnchorIcon } from '../../../img/input/anchor.svg';
import React from 'react';

export const DebouncedSelectFormat = ({ formitem = StyledFormItem, name = 'format', sideTitle, familyId, required, ...props }) => {
  return (
    <DebouncedSelect
      dropdownAlignBottom
      name={name}
      label="Формат"
      rules={{ message: 'Пожалуйста выберите формат', required: required }}
      formitem={{ formitem: formitem }}
      query={QUERY_FORMAT}
      getQueryVariables={(term) => ({ title_Icontains: term, sideTitle: sideTitle, model_Underfamily_Family_Id: familyId })}
      placeholderSpec={{
        svg: AnchorIcon,
        title: 'Формат',
        svgMarginTop: '.06rem',
        needSvgInDropdown: true,
        titleMarginLeft: '-.5rem',
      }}
      valueSelector={(node) => node?.id}
      nodeToTitle={(node) => node?.title}
      queryKey="searchFormatTitles"
      dataPredicate={(data) => (data?.searchFormatTitles?.formatTitles?.edges.length || -1) > 0}
      dataUnpackSpec={{ unpackNodeKey: 'title' }}
      dataUnpack={
        (data) => data?.searchFormatTitles?.formatTitles?.edges
      }
      {...props}
    />
  );
};

export const DebouncedSelectFormatTitle = ({ formitem = StyledFormItem, name = 'format', sideTitle, familyId, required, ...props }) => {
  return (
    <DebouncedSelect
      dropdownAlignBottom
      name={name}
      label="Формат"
      rules={{ message: 'Пожалуйста выберите формат', required: required }}
      formitem={{ formitem: formitem }}
      query={QUERY_FORMAT}
      getQueryVariables={(term) => ({ title_Icontains: term, sideTitle: sideTitle, model_Underfamily_Family_Id: familyId })}
      placeholderSpec={{
        svg: AnchorIcon,
        title: 'Формат',
        svgMarginTop: '.06rem',
        needSvgInDropdown: true,
        titleMarginLeft: '-.5rem',
      }}
      valueSelector={(node) => node?.title}
      nodeToTitle={(node) => node?.title}
      queryKey="searchFormatTitles"
      dataPredicate={(data) => (data?.searchFormatTitles?.formatTitles?.edges.length || -1) > 0}
      dataUnpackSpec={{ unpackNodeKey: 'title' }}
      dataUnpack={
        (data) => data?.searchFormatTitles?.formatTitles?.edges
      }
      {...props}
    />
  );
};
