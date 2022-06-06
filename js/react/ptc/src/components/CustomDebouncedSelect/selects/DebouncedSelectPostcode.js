import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import { QUERY_POST } from '../../../containers/Installations/Orders/queries';
import React from 'react';
import { ReactComponent as MailIcon } from '../../../img/input/mail.svg';

export const DebouncedSelectPostcode = ({ formitem = StyledFormItem, name = 'postcodeId', districtId=undefined, ...props }) => {
  return (
    <DebouncedSelect
      dropdownAlignBottom
      name={name}
      label="Код района"
      formitem={{ formitem: formitem }}
      query={QUERY_POST}
      getQueryVariables={(term) => ({ districtId: districtId, title_Icontains: term })}
      placeholderSpec={{
        svg: MailIcon,
        title: 'Код района',
        svgMarginTop: '.14rem',
        needSvgInDropdown: true,
        titleMarginLeft: '-.5rem',
      }}
      valueSelector={(node) => node?.id}
      queryKey="searchPostcode"
      dataUnpack={(data) =>
        data?.searchPostcode?.edges.filter((node) => node.node.title !== '' && node.node.title !== null)
      }
      {...props}
    />
  );
};
