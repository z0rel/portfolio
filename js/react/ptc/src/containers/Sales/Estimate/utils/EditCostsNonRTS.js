import React, { useContext, useEffect, useRef, useState } from 'react';
import styled from 'styled-components';
import { Form, Input, InputNumber } from 'antd';
import { gql, useMutation } from '@apollo/client';
import { useParams } from 'react-router-dom';
import { EditCostsContext } from './EditCosts';
import { SaveButton } from '../../../../components/Form/FormEdit';
import { FormItemPercentValue } from './FormItemInputDepend';
import { CityFormItem } from './CityFormItem';
import { FormItem } from '../../../../components/Form/FormItem';

import {
  EstimateDrawer,
  fmtPercent,
  fmtPeriod,
  fmtPrice,
  getIdsForMutation,
  getSettedPeriod,
  handleMutationResult,
  StyledPFirst,
  StyledPSecond,
  toNumber,
  toPrice,
} from './utils';
import { getPeriod } from '../../../../components/Logic/getPeriod';
import './editCosts.scss';
import { getConstructionSideCodeByArgs } from '../../../../components/Logic/constructionSideCode';
import { EDIT_ESTIMATE_ITEMS } from '../q_mutations';
import { CustomDateRangePicker } from '../../../../components';
import { useWindowSize } from '../../../../components/Logic/useWindowSize';

export const EditCostsNonRTS = () => {
  const {
    openModal,
    setOpenModal,
    editingItem,
    refetch,
    isEditing,
    title,
    actionTitle,
    selectedRows,
    isPackage,
    blocki,
  } = useContext(EditCostsContext);
  const [confirmLoading, setConfirmLoading] = useState(false);
  const [form] = Form.useForm();
  const [createNonRts] = useMutation(CREATE_NON_RTS_COSTS);
  const [editEstimateItems] = useMutation(EDIT_ESTIMATE_ITEMS);

  const { id, appId } = useParams();
  const params = useParams();

  useEffect(() => {
    if (editingItem && isEditing) {
      setFormValuesNonRts(form, editingItem);
    }
  }, [editingItem, isEditing, form]);

  // Here's how we'll keep track of our component's mounted state
  const componentIsMounted = useRef(true);
  // Using an empty dependency array ensures this only runs on unmount
  useEffect(() => {
    return () => {
      componentIsMounted.current = false;
    };
  }, []);

  return (
    <EstimateDrawer openModal={openModal} setOpenModal={setOpenModal} title={title}>
      <Form
        layout="inline"
        style={{ flexDirection: 'column' }}
        onFinish={(values) => {
          setConfirmLoading(true);
          let [ids, isAll] = getIdsForMutation(isEditing, isPackage, selectedRows, blocki, editingItem);
          if (isEditing || isPackage) {
            let promise;
            promise = editEstimateItems({
              variables: {
                nonRts: getMutationDataNonRtsAdditional(values, isEditing, id, appId),
                ids: ids,
                isAll: isAll,
                projectId: params.id,
                isPackage: isPackage,
              },
            });
            handleMutationResult(promise, {
              form,
              setOpenModal,
              setConfirmLoading,
              refetch,
              isEditing: isEditing || isPackage,
            });
          }
          else {
            handleMutationResult(
              createNonRts({ variables: { input: getMutationDataNonRtsAdditional(values, isEditing, id, appId) } }),
              { form, setOpenModal, setConfirmLoading, refetch, isEditing },
            );
          }
        }}
        form={form}
      >
        <FormInputsNonRTS
          isReservationNonRts={editingItem && editingItem.category === 'nonrtsReservation'}
          componentIsMounted={componentIsMounted}
          confirmLoading={confirmLoading}
          actionTitle={actionTitle}
          isPackage={isPackage}
        />
      </Form>
    </EstimateDrawer>
  );
};

const setFormValuesNonRts = (form, editingItem) => {
  let agPercent = editingItem?.agencyCommission?.toNonrts && editingItem.agencyCommission.percent;
  if (agPercent === false)
    agPercent = null;
  let agSumm = editingItem?.agencyCommission?.toNonrts && editingItem.agencyCommission.value;
  if (agSumm === false)
    agSumm = null;

  form.setFieldsValue({
    inputRent: toPrice(editingItem.rentInput),
    inputTax: toPrice(editingItem.taxInput),
    inputPrint: toPrice(editingItem.printInput),
    inputMount: toPrice(editingItem.mountInput),
    inputCosts: toPrice(editingItem.costsInput),
    inputManufcature: toPrice(editingItem.manufactureInput),
    summRent: toPrice(editingItem.rentSell),
    summTax: toPrice(editingItem.taxSell),
    summPrint: toPrice(editingItem.printSell),
    summMount: toPrice(editingItem.mountSell),
    summManufacture: toPrice(editingItem.manufactureSell),
    summCosts: toPrice(editingItem.costsSell),
    type: editingItem.code,
    count: editingItem.quantity,
    agPercent: agPercent,
    agSumm: agSumm,
    city: editingItem.cityId ? editingItem.cityId : '',
    period: getPeriod(editingItem.period),
  });
};

export const getNonRts = (data, sort = '') => {
  let nonRtsAdditionals = data.additionalNonrts.map((item) => {
    let quantity = item.nonrtsPart.count || 0;
    let agValue = item.agencyCommissionCalculated ?? 0;
    let agPercent =
      item.nonrtsPart.margin && item.agencyCommissionCalculated ? (agValue / item.nonrtsPart.margin) * 100.0 : null;
    if (agPercent === null && item.agencyCommission?.toNonrts)
      agPercent = item.agencyCommission?.percent;
    let sumWithoutAk = (item.nonrtsPart.margin ?? 0) - agValue;

    return {
      key: item.nonrtsPart.id,
      code: item.name,
      city: item.city_Title || '',
      cityId: item.city_Id || '',
      period: fmtPeriod(item.nonrtsPart.startPeriod, item.nonrtsPart.endPeriod),
      quantity: quantity,
      rentInput: fmtPrice(item.nonrtsPart.incomingRent),
      taxInput: fmtPrice(item.nonrtsPart.incomingTax),
      printInput: fmtPrice(item.nonrtsPart.incomingPrinting),
      mountInput: fmtPrice(item.nonrtsPart.incomingInstallation),
      manufactureInput: fmtPrice(item.nonrtsPart.incomingManufacturing),
      costsInput: fmtPrice(item.nonrtsPart.incomingAdditional),
      sumInput: fmtPrice(item.nonrtsPart.pay),
      rentSell: fmtPrice(item.nonrtsPart.saleRent),
      taxSell: fmtPrice(item.nonrtsPart.saleTax),
      printSell: fmtPrice(item.nonrtsPart.salePrinting),
      mountSell: fmtPrice(item.nonrtsPart.saleInstallation),
      manufactureSell: fmtPrice(item.nonrtsPart.saleManufacturing),
      sumSell: fmtPrice(item.nonrtsPart.sale),
      costsSell: fmtPrice(item.nonrtsPart.saleAdditional),
      percentAK: fmtPercent(agPercent),
      sumAK: fmtPrice(agValue),
      margin: fmtPrice(item.nonrtsPart.margin),
      agencyCommission: item.agencyCommission,
      sumWithoutAK: fmtPrice(sumWithoutAk),
      category: 'nonrtsAdditional',
      block: 2,
    };
  });
  let reservationsNonRts = data.reservationsNonrts.map((item) => {
    let agValue = item.agencyCommissionValue ?? 0;
    let agPercent = item.nonrtsPart.margin ? (agValue / item.nonrtsPart.margin) * 100.0 : null;
    if (agPercent === null && item.reservation.agencyCommission?.toNonrts)
      agPercent = item.reservation.agencyCommission?.percent;

    return {
      key: item.reservation.id,
      code: getConstructionSideCodeByArgs(
        item.reservation.constructionSide.postcodeTitle,
        item.reservation.constructionSide.numInDistrict,
        item.reservation.constructionSide.formatCode,
        item.reservation.constructionSide.sideCode,
        item.reservation.constructionSide.advSideCode,
      ),
      city: item.reservation.constructionSide.cityTitle || null,
      cityId: item.reservation.constructionSide.cityId || null,
      period:
        fmtPeriod(item.reservation.dateFrom, item.reservation.dateTo) + ' ' + item.reservation.reservationTypeTitle,
      dateFrom: item.reservation.dateFrom,
      dateTo: item.reservation.dateTo,
      quantity: 1,
      rentInput: fmtPrice(item.nonrtsPart?.incomingRent),
      taxInput: fmtPrice(item.nonrtsPart?.incomingTax),
      printInput: fmtPrice(item.nonrtsPart?.incomingPrinting),
      mountInput: fmtPrice(item.nonrtsPart?.incomingInstallation),
      manufactureInput: fmtPrice(item.nonrtsPart?.incomingManufacturing),
      costsInput: fmtPrice(item.nonrtsPart?.incomingAdditional),
      rentSell: fmtPrice(item.nonrtsPart?.saleRent),
      taxSell: fmtPrice(item.nonrtsPart?.saleTax),
      printSell: fmtPrice(item.nonrtsPart?.salePrinting),
      mountSell: fmtPrice(item.nonrtsPart?.saleInstallation),
      manufactureSell: fmtPrice(item.nonrtsPart?.saleManufacturing),
      costsSell: fmtPrice(item.nonrtsPart?.saleAdditional),
      sumInput: fmtPrice(item.nonrtsPart?.pay),
      sumSell: fmtPrice(item.nonrtsPart?.sale),
      percentAK: fmtPercent(agPercent),
      sumAK: fmtPrice(agValue),
      margin: fmtPrice(item.nonrtsPart?.margin),
      agencyCommission: item.reservation.agencyCommission,
      sumWithoutAK: fmtPrice((item.nonrtsPart?.margin ?? 0) - agValue),
      category: 'nonrtsReservation',
      block: 2,
    };
  });
  let modifiedData = [...reservationsNonRts, ...nonRtsAdditionals];

  switch (sort) {
    case 'abc':
      return modifiedData.sort((a, b) => {
        if (a.city < b.city) {
          return -1;
        }
        if (a.city > b.city) {
          return 1;
        }
        return 0;
      });
    default:
      return modifiedData;
  }
};

const getMutationDataNonRtsAdditional = (values, isEditing, projectId, appendixId) => {
  const [start, end] = getSettedPeriod(values);
  let input = {
    title: values.type,
    count: values.count,
    incomingTax: values.inputTax,
    incomingRent: values.inputRent,
    incomingPrinting: values.inputPrint,
    incomingAdditional: values.inputCosts,
    incomingInstallation: values.inputMount,
    incomingManufacturing: values.inputManufcature,
    saleTax: values.summTax,
    saleRent: values.summRent,
    salePrinting: values.summPrint,
    saleAdditional: values.summCosts,
    saleInstallation: values.summMount,
    saleManufacturing: values.summManufacture,
    startPeriod: start,
    endPeriod: end,
    agencyCommissionValue: values.agSumm === null || values.agSumm === 'null' ? null : toNumber(values.agSumm),
    agencyCommissionPercent:
      values.agPercent === null || values.agPercent === 'null' ? null : toNumber(values.agPercent),
    city: values.city,
  };
  if (!isEditing) {
    if (projectId)
      input.project = projectId;
    if (appendixId)
      input.appendix = appendixId;
  }
  return input;
};

const FormItemZeroInput = ({ name, label, initialValue = 0, ...props }) => {
  return (
    <FormItem name={name} label={label} initialValue={initialValue} {...props}>
      <InputNumber style={{ width: '100%' }} size="large" formatter={(value) => `${value} тг`}/>
    </FormItem>
  );
};

const FormInputsNonRTS = ({ isReservationNonRts, componentIsMounted, confirmLoading, actionTitle, isPackage }) => {
  const [width] = useWindowSize();
  let zival = isPackage ? null : 0;
  return (
    <>
      <StyledPFirst>ВХОДЯЩАЯ СТОИМОСТЬ</StyledPFirst>
      <StyledGrid1Row className="nonrts-1-row">
        <FormItemZeroInput name="inputRent" label="Аренда" initialValue={zival} required={!isPackage}/>
        <FormItemZeroInput name="inputTax" label="Налог" initialValue={zival} required={!isPackage}/>
        <FormItemZeroInput name="inputPrint" label="Печать" initialValue={zival} required={!isPackage}/>
        <FormItemZeroInput name="inputMount" label="Монтаж" initialValue={zival} required={!isPackage}/>
        <FormItemZeroInput name="inputCosts" label="Доп.расходы" initialValue={zival} required={!isPackage}/>
        <FormItemZeroInput name="inputManufcature" label="Производство" initialValue={zival} required={!isPackage}/>
      </StyledGrid1Row>
      <StyledPSecond>СУММА ПРОДАЖИ</StyledPSecond>
      <StyledGrid2Row className="nonrts-2-row">
        <FormItemZeroInput name="summRent" label="Аренда" initialValue={zival} required={!isPackage}/>
        <FormItemZeroInput name="summTax" label="Налог" initialValue={zival} required={!isPackage}/>
        <FormItemZeroInput name="summPrint" label="Печать" initialValue={zival} required={!isPackage}/>
        <FormItemZeroInput name="summMount" label="Монтаж" initialValue={zival} required={!isPackage}/>
        <FormItemZeroInput name="summCosts" label="Доп.расходы" initialValue={zival} required={!isPackage}/>
        <FormItemZeroInput name="summManufacture" label="Производство" initialValue={zival} required={!isPackage}/>
      </StyledGrid2Row>
      <StyledPSecond>ОБЩИЕ ДАННЫЕ</StyledPSecond>
      <StyledGrid3Row className="nonrts-3-row">
        <FormItem name="type" label="Наименование" required={!isPackage}>
          <Input style={{ width: '100%' }} size="large" disabled={isReservationNonRts}/>
        </FormItem>
        <FormItem
          name="period"
          rules={{ message: 'Пожалуйста укажите период.', required: false }}
          label="Период"
          className="form-item-period"
        >
          <CustomDateRangePicker
            placeholder={width > 1945 ? ['Дата начала', 'Дата окончания'] : ['Начало', 'Окончание']}
          />
        </FormItem>
        <CityFormItem
          isReservationNonRts={isReservationNonRts}
          componentIsMounted={componentIsMounted}
          required={false}
        />
        <div className={'two-form-item-block'}>
          <FormItem
            name="count"
            label="Кол-во"
            initialValue={zival}
            className={'two-form-item-1'}
            required={!isPackage}
          >
            <InputNumber style={{ width: '100%' }} size="large" disabled={isReservationNonRts}/>
          </FormItem>
          <FormItemPercentValue
            name1="agPercent"
            label1="Процент АК"
            className1="two-form-item-2"
            name2="agSumm"
            label2="Сумма АК"
            className2="two-form-item-3"
          />
        </div>
        <Form.Item className="editForm-item">
          <SaveButton loading={confirmLoading} actionTitle={actionTitle}/>
        </Form.Item>
      </StyledGrid3Row>
    </>
  );
};

const StyledGrid1Row = styled.div`
  display: grid;
  grid-template-columns: repeat(6, 1fr);
  gap: 30px;
`;

const StyledGrid2Row = styled.div`
  display: grid;
  grid-template-columns: repeat(6, 1fr);
  grid-column-gap: 30px;
  grid-rowgap: 0px;
`;

const StyledGrid3Row = styled.div`
  display: grid;
  grid-template-columns: repeat(6, 1fr);
  column-gap: 30px;
  row-gap: 0px;
  min-height: 123px;
`;

const CREATE_NON_RTS_COSTS = gql`
  mutation addNonRts($input: CreateEstimateNonRtsInput!) {
    createSalesNonrts(input: $input) {
      estimateNonRts {
        id
        title
        count
        incomingTax
        incomingRent
        incomingPrinting
        incomingInstallation
        incomingManufacturing
        city {
          title
        }
      }
    }
  }
`;
