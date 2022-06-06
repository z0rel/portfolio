import React, { useContext, useState } from 'react';
import styled from 'styled-components';
import { EstimateContext } from '../Estimate';
import { ReactComponent as WorldIcon } from '../../../../img/header-bar/world.svg';
import { DebouncedSelect } from '../../../../components/SearchSelect/DebouncedSelect';
import { CITIES_QUERY } from '../q_mutations';
import './estimate.scss';

const StyledDefaultSortParagraph = styled.p`
  font-size: 0.9rem;
  margin-bottom: 0;
  cursor: pointer;
`;

const StyledDefaultSortAY = styled.p`
  font-size: 0.9rem;
  margin-bottom: 16px;
  cursor: pointer;
`;

export const CityFilterDropdown = (props) => {
  const { setSort, sort } = useContext(EstimateContext);
  const [/*selectedCity,*/ setSelectedCity] = useState(null);

  return (
    <div style={{ width: 260 }}>
      <div style={{ padding: '16px' }}>
        <StyledDefaultSortAY
          style={{ color: sort.length ? '#2C5DE5' : '#1A1A1A' }}
          onClick={() => {
            setSort('abc');
            props.clearFilters();
            setSelectedCity(null);
            props.confirm();
          }}
        >
          Сортировать от А до Я
        </StyledDefaultSortAY>
        <StyledDefaultSortParagraph
          style={{ color: sort.length ? '#1A1A1A' : '#2C5DE5' }}
          onClick={() => {
            setSort('');
            props.confirm();
            props.clearFilters();
            setSelectedCity(null);
          }}
        >
          Сортировать по умолчанию
        </StyledDefaultSortParagraph>
      </div>
      <div style={{ borderTop: '1px solid #D3DFF0' }}>
        <div style={{ padding: 16 }}>
          <p style={{ color: '#656565', fontSize: 12 }}>ОПЦИИ</p>
          <DebouncedSelect // Город (CityFilterDropdown) +
            dropdownAlignTop
            disabled={false}
            style={{ width: '100%' }}
            name="city"
            rules="Пожалуйста выберите город."
            label="Город"
            size="middle"
            placeholderSpec={{
              svg: WorldIcon,
              title: 'Выбрать город',
              svgMarginTop: '.15rem',
            }}
            formitem={{ childOnly: false }}
            query={CITIES_QUERY}
            getQueryVariables={(term) => {
              return { title_Icontains: term };
            }}
            valueSelector={(node) => node.title}
            dataPredicate={(data) => data.searchCity.edges && data.searchCity.edges.length > 0}
            dataUnpack={(data) => {
              let arr = [...data.searchCity.edges];
              arr.sort((a, b) => a.node.title.localeCompare(b.node.title));
              return arr;
            }}
            onClear={() => {
              props.clearFilters();
              setSelectedCity(null);
            }}
            onSelect={(val) => {
              props.setSelectedKeys([val]);
              props.confirm();
              setSelectedCity(val);
              setSort('');
            }}
          />
        </div>
      </div>
    </div>
  );
};
