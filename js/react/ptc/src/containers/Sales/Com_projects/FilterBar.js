import React, { useContext } from 'react';
import { comProjectContext } from './Com_projects';

import { FilterMenu280, SearchTitle, FilterText, StyledPanel } from '../../../components/Styles/StyledFilters';
import { Collapse, Form } from 'antd';
import { BtnGroup, ResetButton, SubmitButton } from '../../../components/Styles/ButtonStyles';
import { CustomDateRangePicker } from '../../../components';
import getProjectSelectFilter from '../../../components/Logic/getProjectSelectFilter';

import { DebouncedSelectBrand } from '../../../components/CustomDebouncedSelect/selects/DebouncedSelectBrand';
import { DebouncedSelectClient } from '../../../components/CustomDebouncedSelect/selects/DebouncedSelectClient';
import { DebouncedSelectProjectMulti } from '../../../components/CustomDebouncedSelect/selects/DebouncedSelectProjectMulti';
import { DebouncedSelectAgency } from '../../../components/CustomDebouncedSelect/selects/DebouncedSelectAgency';
import { DebouncedSelectWorkingSector } from '../../../components/CustomDebouncedSelect/selects/DebouncedSelectWorkingSector';
import { DebouncedSelectBackOfficeManager } from '../../../components/CustomDebouncedSelect/selects/DebouncedSelectBackOfficeManager';
import { DebouncedSelectSalesManagerId } from '../../../components/CustomDebouncedSelect/selects/DebouncedSelectSalesManagerId';


const FilterBar = () => {
  const [form] = Form.useForm();
  const [, /*filter*/ setFilter] = useContext(comProjectContext);

  const onFinish = (values) => {
    if (values?.period) {
      let [createdAtGte, createdAtLte] = values?.period;
      delete values['period'];
      values.createdAtGte = createdAtGte;
      values.createdAtGte.set({hour: 0, minute: 0, second: 0})
      values.createdAtLte = createdAtLte;
      values.createdAtLte.set({hour: 23, minute: 59, second: 59})
    }
    console.log(values);
    const project_filter_spec = []
    if ('projectFilter' in values) {
      const v = values['projectFilter'] || [];
      delete values['projectFilter'];
      for (let item of v) {
        let o = getProjectSelectFilter(item);
        project_filter_spec.push({
          projectCode_Iregex:  o?.code_Icontains,
          projectTitle_Iregex: o?.title_Icontains?.replace(/^«/, '^')?.replace(/»$/, '$'),
          fullmatch: o?.title_Icontains?.match(/«.*»/) || false
        })
      }
      values.projectFilterspecOr = project_filter_spec;
    }
    setFilter(values); // console.log(values.date)
  };

  const onReset = () => {
    setFilter({});
    form.resetFields();
  };

  return (
    <FilterMenu280
    // onKeyDown={(e) => {
    //   e.key === 'Enter' && alert('Фильтр');
    // }}
    >
      <SearchTitle>
        <FilterText>Поиск</FilterText>
      </SearchTitle>
      <Form form={form} onFinish={onFinish}>
        <Collapse expandIconPosition="right">
          <StyledPanel header="По дате" key="1">
            <Form.Item name="period">
              <CustomDateRangePicker placeholder={['Начало С', 'Начало По']} />
            </Form.Item>
          </StyledPanel>
          <StyledPanel header="По параметрам" key="2">
            <DebouncedSelectProjectMulti />
            <DebouncedSelectBrand />
            <DebouncedSelectClient />
            <DebouncedSelectAgency />
            <DebouncedSelectWorkingSector />
            <DebouncedSelectBackOfficeManager />
            <DebouncedSelectSalesManagerId />
          </StyledPanel>
        </Collapse>
        <BtnGroup>
          <SubmitButton htmlType="submit">Поиск</SubmitButton>
          <ResetButton onClick={onReset}>Очистить</ResetButton>
        </BtnGroup>
      </Form>
      <style>
        {`
        .ant-collapse-content{
           background-color: #f5f7fa !important;
        }`}
      </style>
    </FilterMenu280>
  );
};

export default FilterBar;
