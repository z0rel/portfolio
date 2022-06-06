import React from 'react';
import { GET_ADVERTISER, GET_BRANDS, GET_MANAGERS, GET_WORK_SECTOR } from './queries';
import { useQuery } from '@apollo/client';
import { StyledSelect } from '../../../../components/Styles/DesignList/styles';

export const ManagersInput = ({ Item, name }) => {
  const managers = useQuery(GET_MANAGERS);
  let managersData = managers && managers.data ? managers.data.searchUser.edges : null;
  return (
    <Item name={name}>
      <StyledSelect showSearch>
        {managersData &&
          managersData.map((item) => {
            return (
              <StyledSelect.Option key={item.node.id} value={item.node.id}>
                <span>{item.node.firstName + ' ' + item.node.lastName}</span>
              </StyledSelect.Option>
            );
          })}
      </StyledSelect>
    </Item>
  );
};

export const BrandInput = ({ Item, name, form }) => {
  const brands = useQuery(GET_BRANDS);
  let brandsData = brands && brands.data ? brands.data.searchBrand.edges : null;

  return (
    <Item name={name}>
      <StyledSelect
        showSearch
        onChange={(e) => {
          const sectors = brandsData.filter((brand) => {
            return e === brand.node.id;
          });
          const workingSector = sectors.length ? sectors[0].node.workingSector.id : '';

          form.setFieldsValue({
            workingSector,
          });
        }}>
        {brandsData &&
          brandsData.map((item) => {
            return (
              <StyledSelect.Option key={item.node.id} value={item.node.id}>
                <span>{item.node.title}</span>
              </StyledSelect.Option>
            );
          })}
      </StyledSelect>
    </Item>
  );
};

export const WorkingSectorInput = ({ Item, name }) => {
  const workSec = useQuery(GET_WORK_SECTOR);
  let workSecData = workSec && workSec.data ? workSec.data.searchWorkingSector.edges : null;
  return (
    <Item name={name}>
      <StyledSelect showSearch>
        {workSecData &&
          workSecData.map((item) => {
            return (
              <StyledSelect.Option value={item.node.id} key={item.node.id}>
                <span>{item.node.title}</span>
              </StyledSelect.Option>
            );
          })}
      </StyledSelect>
    </Item>
  );
};

export const AdvInput = ({ Item, name }) => {
  const advert = useQuery(GET_ADVERTISER);
  let advertData = advert && advert.data ? advert.data.searchPartner.edges : null;

  return (
    <Item name={name}>
      <StyledSelect showSearch>
        {advertData &&
          advertData.map((item) => {
            return (
              <StyledSelect.Option key={item.node.id} value={item.node.id}>
                <span>{item.node.title}</span>
              </StyledSelect.Option>
            );
          })}
      </StyledSelect>
    </Item>
  );
};
