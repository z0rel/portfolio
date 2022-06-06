import React, { useContext } from 'react';
import { invoiceContext } from './Invoice';
import { FilterMenu, SearchTitle, FilterText, StyledPanel } from '../../../components/Styles/StyledFilters';
import { Select, Collapse, DatePicker, Form } from 'antd';
import { BtnGroup, ResetButton, SubmitButton } from '../../../components/Styles/ButtonStyles';
const { Option } = Select;

const FilterBar = () => {
  const [form] = Form.useForm();
  const [filter, setFilter] = useContext(invoiceContext);
  const onFinish = (values) => {
    setFilter(values);

    console.log(filter);
  };

  const onReset = () => {
    form.resetFields();
  };

  return (
    <FilterMenu
      onKeyDown={(e) => {
        e.key === 'Enter' && alert('Фильтр');
      }}
    >
      <SearchTitle>
        <FilterText>Поиск</FilterText>
      </SearchTitle>
      <Form form={form} onFinish={onFinish}>
        <Collapse expandIconPosition={'right'}>
          <StyledPanel header="По дате" key="1">
            <Form.Item name="date">
              <DatePicker placeholder="01/01/2020" size={'large'} format="DD/MM/YYYY" style={{ width: '100%' }} />
            </Form.Item>
          </StyledPanel>
          <StyledPanel header="По параметрам" key="2">
            <Form.Item name="projectCode">
              <Select placeholder="Код проекта  " size={'large'}>
                <Option value="case 1">case 1</Option>
                <Option value="case 2">case 2</Option>
              </Select>
            </Form.Item>
            <Form.Item name="appendixNumber">
              <Select placeholder="Номер приложения" size={'large'}>
                <Option value="case 1">case 1</Option>
                <Option value="case 2">case 2</Option>
              </Select>
            </Form.Item>
            <Form.Item name="brand">
              <Select placeholder="Бренд" size={'large'}>
                <Option value="case 1">case 1</Option>
                <Option value="case 2">case 2</Option>
              </Select>
            </Form.Item>
            <Form.Item name="advertiser">
              <Select placeholder="Рекламодатель" size={'large'}>
                <Option value="case 1">case 1</Option>
                <Option value="case 2">case 2</Option>
              </Select>
            </Form.Item>
            <Form.Item name="advAgency">
              <Select placeholder="Рекламное агенство " size={'large'}>
                <Option value="case 1">case 1</Option>
                <Option value="case 2">case 2</Option>
              </Select>
            </Form.Item>
            <Form.Item name="respManager">
              <Select placeholder="Ответств. менеджер" size={'large'}>
                <Option value="case 1">case 1</Option>
                <Option value="case 2">case 2</Option>
              </Select>
            </Form.Item>
            <Form.Item name="advManager">
              <Select placeholder="Менеджер по продажам" size={'large'}>
                <Option value="case 1">case 1</Option>
                <Option value="case 2">case 2</Option>
              </Select>
            </Form.Item>
          </StyledPanel>
        </Collapse>
        <BtnGroup>
          <SubmitButton onClick={() => alert('Фильтр')}>Поиск</SubmitButton>
          <ResetButton onClick={onReset}>Очистить</ResetButton>
        </BtnGroup>
      </Form>
      <style>
        {`
        .ant-collapse-content{
           background-color: #f5f7fa !important;
        }
        `}
      </style>
    </FilterMenu>
  );
};

export default FilterBar;
