import { StyledFormItem } from './StyledFormItem';
import { DebouncedSelect } from '../../SearchSelect/DebouncedSelect';
import { ReactComponent as AnchorIcon } from '../../../img/input/anchor.svg';
import { gql } from '@apollo/client';

export const DebouncedSelectAdvertisingSide = ({ formitem = StyledFormItem, name = 'advertisingSide', side_Id, format_id, side_size_title, required, ...props }) => {
  return (
    <DebouncedSelect
      dropdownAlignBottom
      name={name}
      label="Рекламная сторона"
      rules={{ message: 'Пожалуйста выберите рекламную сторону', required: required }}
      formitem={{ formitem: formitem }}
      query={QUERY_ADV_SIDE}
      getQueryVariables={(term) => ({ title_Icontains: term, side_Id: side_Id, format_id: format_id, side_size_title: side_size_title })}
      placeholderSpec={{
        svg: AnchorIcon,
        title: 'Рекламная сторона',
        svgMarginTop: '.3rem',
        needSvgInDropdown: true,
        titleMarginLeft: '-.5rem',
      }}
      valueSelector={(node) => node?.id}
      nodeToTitle={(node) => node?.title}
      queryKey="searchAdvertisingSide"
      dataPredicate={(data) => (data?.searchAdvertisingSide?.edges.length || -1) > 0}
      dataUnpackSpec={{ unpackNodeKey: 'title' }}
      dataUnpack={(data) => data?.searchAdvertisingSide?.edges}
      {...props}
    />
  );
};

export const QUERY_ADV_SIDE = gql`
query SearchAdvertisingSide($title_Icontains: String, $side_Id: ID, $title: String) {
  searchAdvertisingSide(side_Id: $side_Id, title_Icontains: $title_Icontains, title: $title ) {
    edges {
      node {
        id
        title
      }
    }
  }
}
`;
