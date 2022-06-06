import React, { useContext, useState } from 'react';
import styled from 'styled-components';
import { Checkbox, Collapse, Form } from 'antd';
import { FilterMenu280, FilterText, SearchTitle, StyledPanel } from '../../../components/Styles/StyledFilters';
import { BtnGroup, ResetButton, SubmitButton } from '../../../components/Styles/ButtonStyles';
import { CustomDateRangePicker, CustomInput } from '../../../components';
import { ReactComponent as CityIcon } from '../../../img/input/city.svg';
import { ReactComponent as DistrictIcon } from '../../../img/input/district.svg';
import { ReactComponent as ConstructionIcon } from '../../../img/input/construction.svg';
import { ReactComponent as PhoneIcon } from '../../../img/input/phone.svg';
import { ReactComponent as ArrowsIcon } from '../../../img/input/arrows.svg';
import { FormItem } from '../../../components/Form/FormItem';
import { DebouncedSelect } from '../../../components/SearchSelect/DebouncedSelect';
import { adverContext } from './AdvertisingParties';

import './styles/styles_adv_part.scss';

import {
  QUERY_CITIES,
  QUERY_DISTRICTS,
  QUERY_FAMILIES,
  QUERY_FORMAT,
  QUERY_MARKETING_ADDRESS,
  QUERY_SEARCH_PARTNER,
  QUERY_SIDE,
  QUERY_SIZE,
} from './q_filter_queries';

const StyledFormItemCheckbox = styled(Form.Item)`
  margin: 0;
  padding: 0;
`;

const FilterBar = ({ refetch, ganttUpdater }) => {
  const [form] = Form.useForm();
  const { setFilter, setPeriod } = useContext(adverContext);
  const [cityId, setCityId] = useState(null);
  const [districtId, setDistrictId] = useState(null);
  const [familyId, setFamilyId] = useState(null);
  const [formatTitle, setFormatTitle] = useState(null);
  const [sideTitle, setSideTitle] = useState(null);
  const [sideSize, setSideSize] = useState(null);

  const componentIsMounted = { current: true };

  const clearForm = () => {
    form.resetFields();
    setCityId(null);
    setDistrictId(null);
    setFamilyId(null);
    setFormatTitle(null);
    setSideTitle(null);
    setSideSize(null);
  };

  return (
    <FilterMenu280
      onKeyDown={(e) => {
        e.key === 'Enter' && alert('Фильтр');
      }}
    >
      <SearchTitle>
        <FilterText>Поиск</FilterText>
      </SearchTitle>
      <Form
        form={form}
        onFinish={(v) => onFinish(v, formatTitle, sideTitle, sideSize, setFilter, setPeriod, refetch, ganttUpdater)}
      >
        <Collapse expandIconPosition={'right'} defaultActiveKey={['1']}>
          <StyledPanel header="По дате" key="1">
            <Form.Item name="date">
              <CustomDateRangePicker/>
            </Form.Item>
          </StyledPanel>
          <StyledPanel header="Статус брони" key="2">
            <StyledFormItemCheckbox name="statusFree" valuePropName="checked">
              <Checkbox defaultChecked>
                <div className="dot-1" style={{ marginRight: '.8rem' }}/>
                <span>Свободно</span>
              </Checkbox>
            </StyledFormItemCheckbox>
            <StyledFormItemCheckbox name="statusReserved" valuePropName="checked">
              <Checkbox defaultChecked>
                <div className="dot-2" style={{ marginRight: '.8rem' }}/>
                Забронировано
              </Checkbox>
            </StyledFormItemCheckbox>
            <StyledFormItemCheckbox name="statusApproved" valuePropName="checked">
              <Checkbox defaultChecked>
                <div className="dot-3" style={{ marginRight: '.8rem' }}/>
                Утверждено
              </Checkbox>
            </StyledFormItemCheckbox>
            <StyledFormItemCheckbox name="statusSaled" valuePropName="checked">
              <Checkbox defaultChecked>
                <div className="dot-4" style={{ marginRight: '.8rem' }}/>
                Продано
              </Checkbox>
            </StyledFormItemCheckbox>
            <StyledFormItemCheckbox name="statusUnavailable" valuePropName="checked">
              <Checkbox defaultChecked>
                <div className="dot-4 dot-unavailable" style={{ marginRight: '.8rem' }}/>
                Недоступно
              </Checkbox>
            </StyledFormItemCheckbox>
            <StyledFormItemCheckbox name="statusAll" valuePropName="checked">
              <Checkbox defaultChecked>
                <div className="dot-4 dot-all" style={{ marginRight: '.8rem' }}/>
                Все
              </Checkbox>
            </StyledFormItemCheckbox>
          </StyledPanel>
          <StyledPanel header="По адресу" key="3">
            <DebouncedSelect // Город +
              dropdownAlignBottom
              name="city"
              label="Город"
              formitem={{ formitem: StyledFormItem }}
              query={QUERY_CITIES}
              getQueryVariables={(term) => {
                return { title_Icontains: term };
              }}
              placeholderSpec={{
                svg: CityIcon,
                title: 'Выбрать город',
                svgMarginTop: 0,
                needSvgInDropdown: true,
                titleMarginLeft: '-.5rem',
              }}
              componentIsMounted={componentIsMounted}
              valueSelector={(node) => node?.id}
              queryKey="searchCity"
              handleValueChanged={(value) => setCityId(value)}
              dataUnpackSpec={{ unpackNodeKey: 'title' }}
            />
            <DebouncedSelect // Район +
              dropdownAlignBottom
              name="district"
              label="Район"
              formitem={{ formitem: StyledFormItem }}
              query={QUERY_DISTRICTS}
              getQueryVariables={(term) => {
                return { city_Id: cityId, title_Icontains: term };
              }}
              placeholderSpec={{
                svg: DistrictIcon,
                title: 'Выбрать район',
                svgMarginTop: '.14rem',
                needSvgInDropdown: true,
                titleMarginLeft: '-.5rem',
              }}
              componentIsMounted={componentIsMounted}
              valueSelector={(node) => node?.id}
              queryKey="searchDistrict"
              handleValueChanged={(value) => setDistrictId(value)}
              dataUnpack={(data) => {
                return data?.searchDistrict?.edges.filter((node) => node.node.title !== '' && node.node.title !== null);
              }}
            />
            <DebouncedSelect // Маркетинговый адрес +
              dropdownAlignBottom
              name="marketingAddress"
              label="Маркетинговый адрес"
              formitem={{ formitem: StyledFormItem }}
              query={QUERY_MARKETING_ADDRESS}
              getQueryVariables={(term) => {
                return { city_Id: cityId, district_Id: districtId, address_Icontains: term };
              }}
              placeholderSpec={{
                svg: DistrictIcon,
                title: 'Маркетинговый адрес',
                svgMarginTop: '.14rem',
                needSvgInDropdown: true,
                titleMarginLeft: '-.5rem',
              }}
              componentIsMounted={componentIsMounted}
              valueSelector={(node) => node?.id}
              queryKey="searchLocAdress"
              dataUnpackSpec={{ unpackNodeKey: 'address' }}
              dataUnpack={(data) => {
                return data?.searchLocAdress?.edges.filter(
                  (node) => node.node.address !== '' && node.node.address !== null,
                );
              }}
            />
          </StyledPanel>
          <StyledPanel header="По параметрам" key="4">
            <DebouncedSelect // Семейство конструкции
              dropdownAlignBottom
              name="family"
              label="Семейство конструкции"
              formitem={{ formitem: StyledFormItem }}
              query={QUERY_FAMILIES}
              getQueryVariables={(term) => {
                return { title_Icontains: term };
              }}
              placeholderSpec={{
                svg: ConstructionIcon,
                title: 'Семейство',
                svgMarginTop: '.1rem',
                needSvgInDropdown: true,
                titleMarginLeft: '-.5rem',
              }}
              componentIsMounted={componentIsMounted}
              valueSelector={(node) => node?.id}
              handleValueChanged={(value, values) => {
                form.setFields([
                  { name: 'format', value: null },
                  { name: 'side', value: null },
                  { name: 'size', value: null },
                ]);
                setFamilyId(value);
                setFormatTitle(null);
                setSideSize(null);
                setSideTitle(null);
              }}
              queryKey="searchFamilyConstruction"
              dataUnpackSpec={{ unpackNodeKey: 'title' }}
            />
            <DebouncedSelect // Формат конструкции
              dropdownAlignBottom
              name="format"
              label="Формат кострукции"
              formitem={{ formitem: StyledFormItem }}
              query={QUERY_FORMAT}
              getQueryVariables={(term) => {
                return { model_Underfamily_Family_Id: familyId, title_Icontains: term, sideTitle: sideTitle };
              }}
              dataPredicate={(data) => (data?.searchFormatTitles?.formatTitles?.edges.length || -1) > 0}
              placeholderSpec={{
                svg: PhoneIcon,
                title: 'Формат',
                svgMarginTop: '.06rem',
                needSvgInDropdown: true,
                titleMarginLeft: '-.5rem',
              }}
              componentIsMounted={componentIsMounted}
              valueSelector={(node) => node?.id}
              nodeToTitle={(node) => node?.title}
              handleValueChanged={(value, values) => {
                let title = values.get(value)?.title || null;
                setFormatTitle(title);
                form.setFields([
                  { name: 'side', value: null },
                  { name: 'size', value: null },
                ]);
                setSideSize(null);
                setSideTitle(null);
              }}
              queryKey="searchFormatTitles"
              dataUnpackSpec={{ unpackNodeKey: 'title' }}
              dataUnpack={(data) => data?.searchFormatTitles?.formatTitles?.edges}
            />
            <DebouncedSelect // Стророна конструкции
              dropdownAlignBottom
              name="side"
              label="Стророна кострукции"
              formitem={{ formitem: StyledFormItem }}
              query={QUERY_SIDE}
              getQueryVariables={(term) => {
                return {
                  format_Model_Underfamily_Family_Id: familyId,
                  format_Title: formatTitle,
                  title_Icontains: term,
                  size: sideTitle,
                };
              }}
              dataPredicate={(data) => (data?.searchSideTitles?.sideSize?.edges.length || -1) > 0}
              placeholderSpec={{
                svg: ArrowsIcon,
                title: 'Стророна',
                svgMarginTop: '.3rem',
                needSvgInDropdown: true,
                titleMarginLeft: '-.5rem',
              }}
              componentIsMounted={componentIsMounted}
              valueSelector={(node) => node?.id}
              nodeToTitle={(node) => node?.title}
              handleValueChanged={(value, values) => {
                let title = values.get(value)?.title || null;
                let size = values.get(value)?.size || null;
                form.setFields([{ name: 'size', value: size }]);
                setSideTitle(title);
                setSideSize(null);
              }}
              queryKey="searchSideTitles"
              dataUnpackSpec={{ unpackNodeKey: 'title' }}
              dataUnpack={(data) => data?.searchSideTitles?.sideSize?.edges}
            />
            <DebouncedSelect // Размер конструкции
              dropdownAlignBottom
              name="size"
              label="Размер кострукции"
              formitem={{ formitem: StyledFormItem }}
              query={QUERY_SIZE}
              getQueryVariables={(term) => {
                return {
                  format_Model_Underfamily_Family_Id: familyId,
                  format_Title: formatTitle,
                  title: sideTitle,
                  size_Icontains: term,
                };
              }}
              dataPredicate={(data) => (data?.searchSideSize?.sideSize?.edges.length || -1) > 0}
              placeholderSpec={{
                svg: ArrowsIcon,
                title: 'Размер',
                svgMarginTop: '.3rem',
                needSvgInDropdown: true,
                titleMarginLeft: '-.5rem',
              }}
              componentIsMounted={componentIsMounted}
              valueSelector={(node) => node?.id}
              // nodeToTitle={node => node?.size}
              handleValueChanged={(value, values) => {
                let size = values.get(value)?.size;
                setSideSize(size);
              }}
              queryKey="searchSideSize"
              dataUnpackSpec={{ unpackNodeKey: 'size' }}
              dataUnpack={(data) => data?.searchSideSize?.sideSize?.edges}
            />
            <DebouncedSelect // Владелец конструкции
              dropdownAlignBottom
              name="owner"
              label="Владелец"
              formitem={{ formitem: StyledFormItem }}
              query={QUERY_SEARCH_PARTNER}
              getQueryVariables={(term) => {
                return { title_Icontains: term };
              }}
              placeholderSpec={{
                svg: ArrowsIcon,
                title: 'Владелец',
                svgMarginTop: '.3rem',
                needSvgInDropdown: true,
                titleMarginLeft: '-.5rem',
              }}
              componentIsMounted={componentIsMounted}
              valueSelector={(node) => node?.id}
              emptyrowTitle={'РТС'}
              queryKey="searchPartner"
              dataUnpackSpec={{ unpackNodeKey: 'title' }}
            />
            <StyledFormItem name="sidecode" label="Код стороны" required={false}>
              <CustomInput placeholder="050000.000000.FMT.A.A1"/>
            </StyledFormItem>

            <StyledFormItemCheckbox name="statusConnection" valuePropName="checked">
              <Checkbox defaultChecked>Освещение</Checkbox>
            </StyledFormItemCheckbox>
          </StyledPanel>
        </Collapse>
        <BtnGroup>
          <SubmitButton htmlType="submit">Поиск</SubmitButton>
          <ResetButton onClick={() => clearForm()}>Очистить</ResetButton>
        </BtnGroup>
      </Form>
      <style>
        {`
        .ant-collapse-content{
           background-color: #f5f7fa !important;
        }
        `}
      </style>
    </FilterMenu280>
  );
};

const StyledFormItem = styled(FormItem)`
  .ant-form-item-label {
    label {
      margin-bottom: 0;
    }
  }
  margin-bottom: 0.6rem;
`;

export default FilterBar;

const parseConstructionSideCode = (sidecode) => {
  let result = {
    postcode_Icontains: null,
    numInDistrict: null,
    codeFormat_Icontains: null,
    codeSide: null,
    codeAdvSide: null,
  };
  if (!sidecode)
    return result;

  let m = /([0-9]*)(\.([0-9]*)(\.([A-Za-z0-9_-]*)?(\.([A-Za-z0-9_-]*)(\.([A-Za-z0-9_-]*))?)?)?)?/.exec(sidecode);
  result.postcode_Icontains = m[1];
  result.numInDistrict = m[3] ? parseInt(m[3]) : null;
  result.codeFormat_Icontains = m[5];
  result.codeSide = m[7];
  result.codeAdvSide = m[9];
  return result;
};

const onFinish = (_values, formatTitle, sideTitle, sideSize, setFilter, setPeriod, refetch, ganttUpdater) => {
  let values = { ..._values };

  let dstFilter = { ...parseConstructionSideCode(_values.sidecode) };
  if (values && values.date && values.date.length === 2) {
    let startYear = values.date[0].format('YYYY');
    let startMonth = values.date[0].format('M') - 1;
    let startDay = values.date[0].format('D');
    let endYear = values.date[1].format('YYYY');
    let endMonth = values.date[1].format('M') - 1;
    let endDay = values.date[1].format('D');
    setPeriod({
      start: new Date(startYear, startMonth, startDay - 1, 0, 0, 0, 0),
      end: new Date(endYear, endMonth, endDay - 1, 0, 0, 0, 0),
    });
  }
  let anyNonfree = values.statusSaled || values.statusUnavailable || values.statusApproved || values.statusReserved;
  let onlyNonfree = !values.statusFree && !values.statusAll && anyNonfree;
  if (onlyNonfree) {
    dstFilter.reservationType = [
      values.statusSaled && 'Продано',
      values.statusUnavailable && 'Недоступно',
      values.statusApproved && 'Утверждено',
      values.statusReserved && 'Забронировано',
    ].filter((v) => v);
    if (dstFilter.reservationType.length === 1)
      dstFilter.reservationType = dstFilter.reservationType[0];
    else if (dstFilter.reservationType.length > 1)
      dstFilter.reservationType = '(' + dstFilter.reservationType.join('|') + ')';
  }
  if (values.city)
    dstFilter.city = values.city;
  if (values.district)
    dstFilter.district = values.district;
  if (values.family)
    dstFilter.family = values.family;

  if (formatTitle)
    dstFilter.format = formatTitle;

  if (sideTitle)
    dstFilter.side = sideTitle;

  if (sideSize)
    dstFilter.size = sideSize;

  if (values.statusConnection)
    dstFilter.statusConnection = values.statusConnection;
  if (values.owner && values.owner !== 'РТС')
    dstFilter.owner = values.owner;
  if (values.marketingAddress)
    dstFilter.marketingAddress = values.marketingAddress;

  values.dstFilter = dstFilter;
  setFilter(values);

  if (refetch)
    refetch();

  if (ganttUpdater)
    ganttUpdater(null);
};
