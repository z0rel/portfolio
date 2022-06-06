import React, { useContext, useEffect, useRef, useState } from 'react';
import { Form, Input, InputNumber } from 'antd';
import {
  agencyComissionDistributed,
  EstimateDrawer,
  fmtPercent,
  fmtPeriod,
  fmtPrice,
  getIdsForMutation,
  getSettedPeriod,
  handleMutationResult,
  nullFormatterPercent,
  toNumber,
  toPrice,
} from './utils';
import { gql, useMutation } from '@apollo/client';
import { useParams } from 'react-router-dom';
import { EditCostsContext } from './EditCosts';
import { getPeriod } from '../../../../components/Logic/getPeriod';
import { SaveButton } from '../../../../components/Form/FormEdit';
import { FormItemPercentValue } from './FormItemInputDepend';
import { CityFormItem } from './CityFormItem';
import { FormItem } from '../../../../components/Form/FormItem';
import moment from 'moment';
import { EDIT_ESTIMATE_ITEMS } from '../q_mutations';
import { CustomDateRangePicker } from '../../../../components';
import { useWindowSize } from '../../../../components/Logic/useWindowSize';

import { estimate } from '../../../../assets/proto_compiled/proto';

const AdditionalRtsCategory = estimate.AdditionalRtsCategory;

export const EditCostsRtsAdditional = () => {
  const [form] = Form.useForm();
  const [createAddCosts] = useMutation(CREATE_ADDITIONAL_COSTS);
  const [confirmLoading, setConfirmLoading] = useState(false);
  const [editEstimateItems] = useMutation(EDIT_ESTIMATE_ITEMS);
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
  const params = useParams();

  useEffect(() => {
    if (editingItem && isEditing) {
      setFormValuesRtsAdditional(form, editingItem);
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
  const { id, appId } = useParams();

  return (
    <EstimateDrawer openModal={openModal} setOpenModal={setOpenModal} title={title}>
      <Form
        layout="inline"
        style={{ flexDirection: 'row' }}
        onFinish={(values) => {
          let [ids, isAll] = getIdsForMutation(isEditing, isPackage, selectedRows, blocki, editingItem);
          if (isEditing || isPackage) {
            handleMutationResult(
              editEstimateItems({
                variables: {
                  rtsAdditional: getMutationDataRtsAdditional(values, editingItem, isEditing, id, appId),
                  ids: ids,
                  isAll: isAll,
                  projectId: params.id,
                  isPackage: isPackage,
                },
              }),
              { form, setOpenModal, setConfirmLoading, refetch, isEditing: isEditing || isPackage },
            );
          }
          else {
            handleMutationResult(
              createAddCosts({
                variables: {
                  input: getMutationDataRtsAdditional(values, editingItem, isEditing, id, appId),
                },
              }),
              { form, setOpenModal, setConfirmLoading, refetch, isEditing },
            );
          }
        }}
        form={form}
      >
        <FormInputsRtsAdditional
          componentIsMounted={componentIsMounted}
          confirmLoading={confirmLoading}
          actionTitle={actionTitle}
          isPackage={isPackage}
        />
      </Form>
    </EstimateDrawer>
  );
};

const setFormValuesRtsAdditional = (form, editingItem) => {
  let agSum = null;
  let agPercent = null;
  if (editingItem.agencyCommission) {
    let ak = editingItem.agencyCommission;
    let cat = editingItem.category;
    if (agencyComissionDistributed(ak, cat)) {
      agSum = ak.value;
      agPercent = ak.percent;
    }
  }
  form.setFieldsValue({
    name: editingItem.nameOfService || '',
    count: toPrice(editingItem.quantity),
    price: toPrice(editingItem.price),
    discount: toPrice(editingItem.discount),
    agPercent: agPercent === null ? null : toPrice(agPercent),
    agSumm: agSum === null ? null : toPrice(agSum),
    city: editingItem.cityId ? editingItem.cityId : '',
    period: getPeriod(editingItem.period),
  });
};

export const getRtsAdditional = (data, sort = '', period = '') => {
  let modifiedData = data.additionalRts.map((item) => {
    return {
      id: item.id || '',
      key: item.id || '',
      nameOfService: item.title || '',
      city: item.cityTitle || '',
      cityId: item.cityId || '',
      period: fmtPeriod(item.startPeriod, item.endPeriod),
      quantity: item.count || '',
      price: fmtPrice(item.price),
      discount: fmtPercent(item.discountPercent),
      priceAfterDiscount: fmtPrice(item.priceAfterDiscount),
      sum: fmtPrice(item.summaAfterDiscount),
      percentAK: fmtPercent(item.agencyCommissionPercent),
      sumAK: fmtPrice(item.agencyCommissionValue),
      sumWithoutAK: fmtPrice(item.valueWithoutAgencyCommission),
      agencyCommission: item.agencyCommission,
      category: item.category,
      block: 1,
    };
  });
  switch (sort) {
    case 'abc':
      modifiedData = modifiedData.sort((a, b) => {
        if (a.city < b.city) {
          return -1;
        }
        if (a.city > b.city) {
          return 1;
        }
        return 0;
      });
      break;
    default:
      break;
  }
  switch (period) {
    case 'increase':
      modifiedData = modifiedData
        .sort((a, b) => {
          const START = moment(a.period.split(' - ')[0], 'DD.MM.YYYY');
          const END = moment(a.period.split(' - ')[1], 'DD.MM.YYYY');
          const START2 = moment(b.period.split(' - ')[0], 'DD.MM.YYYY');
          const END2 = moment(b.period.split(' - ')[1], 'DD.MM.YYYY');
          const duration = moment.duration(END.diff(START)).asDays();
          const duration2 = moment.duration(END2.diff(START2)).asDays();
          return duration - duration2;
        })
        .reverse();
      break;
    case 'decrease':
      modifiedData = modifiedData.sort((a, b) => {
        const START = moment(a.period.split(' - ')[0], 'DD.MM.YYYY');
        const END = moment(a.period.split(' - ')[1], 'DD.MM.YYYY');
        const START2 = moment(b.period.split(' - ')[0], 'DD.MM.YYYY');
        const END2 = moment(b.period.split(' - ')[1], 'DD.MM.YYYY');
        const duration = moment.duration(END.diff(START)).asDays();
        const duration2 = moment.duration(END2.diff(START2)).asDays();
        return duration - duration2;
      });
      break;
    default:
      break;
  }

  return modifiedData;
};

const getMutationDataRtsAdditional = (values, editingItem, isEditing, projectId, appendixId) => {
  const [start, end] = getSettedPeriod(values);
  let ak = editingItem?.agencyCommission;
  let input = {
    title: values.name,
    count: values.count,
    discountPercent: values.discount,
    price: values.price,
    agencyCommission: {
      value: values.agSumm === 'null' ? null : toNumber(values.agSumm),
      percent: values.agPercent === 'null' ? null : toNumber(values.agPercent),
    },
    city: values.city,
    startPeriod: start,
    endPeriod: end,
  };
  if (ak) {
    switch (editingItem?.category) {
      case AdditionalRtsCategory.NALOG:
        input.agencyCommission.toNalog = true;
        break;
      case AdditionalRtsCategory.MOUNTING:
        input.agencyCommission.toMount = true;
        break;
      case AdditionalRtsCategory.PRINTING:
        input.agencyCommission.toPrint = true;
        break;
      case AdditionalRtsCategory.RENT:
        input.agencyCommission.toRent = true;
        break;
      case AdditionalRtsCategory.ADDITIONAL:
        input.agencyCommission.toAdditional = true;
        break;
      default:
        break;
    }
  }
  if (!isEditing) {
    if (projectId)
      input.project = projectId;
    if (appendixId)
      input.appendix = appendixId;
  }
  return input;
};

const FormInputsRtsAdditional = ({ componentIsMounted, confirmLoading, actionTitle, isPackage }) => {
  const [width] = useWindowSize();
  const datePlaceholderLong = ['Дата начала', 'Дата окончания'];
  const datePlaceholderShort = ['Начало', 'Окончание'];
  return (
    <div className={'form-input-rts-additional'}>
      <FormItem
        name="name"
        rules={{ message: 'Пожалуйста введите наименование услуги.', required: !isPackage }}
        label="Наименование услуги"
        className="service-name"
      >
        <Input size="large"/>
      </FormItem>
      <CityFormItem required={false} isReservationNonRts={false} componentIsMounted={componentIsMounted}/>
      <FormItem
        name="period"
        rules={{ message: 'Пожалуйста укажите период.', required: false }}
        label="Период"
        className={'period-input'}
      >
        <CustomDateRangePicker
          placeholder={
            width > 2176
              ? datePlaceholderLong
              : width > 1997
              ? datePlaceholderShort
              : width > 1628
                ? datePlaceholderLong
                : datePlaceholderShort
          }
        />
      </FormItem>
      <FormItem
        name="count"
        rules={{ message: 'Пожалуйста введите количество.', required: !isPackage }}
        label="Кол-во"
        initialValue={isPackage ? null : 0}
      >
        <InputNumber type="number" size="large"/>
      </FormItem>
      <FormItem
        name="price"
        rules={{ message: 'Пожалуйста введите цену.', required: !isPackage }}
        label="Цена"
        initialValue={isPackage ? null : 0}
      >
        <InputNumber size="large" formatter={(value) => `${value} тг`}/>
      </FormItem>
      <FormItem name="discount" label="Скидка" initialValue={null} required={false}>
        <InputNumber size="large" formatter={nullFormatterPercent}/>
      </FormItem>
      <FormItemPercentValue name1="agPercent" label1="Процент АК" name2="agSumm" label2="Сумма АК"/>
      <Form.Item className="editForm-item save-button">
        <SaveButton loading={confirmLoading} actionTitle={actionTitle}/>
      </Form.Item>
    </div>
  );
};

const CREATE_ADDITIONAL_COSTS = gql`
  mutation createAdditionalCost($input: CreateAdditionalCostsInput!) {
    createSalesAdditionalCost(input: $input) {
      additionalCosts {
        id
        title
        startPeriod
        endPeriod
        count
        discountPercent
        price
        count
        city {
          title
        }
      }
    }
  }
`;
