import { Form } from 'antd';
import { useMutation } from '@apollo/client';
import React, { useContext, useEffect, useRef, useState } from 'react';
import { EditCostsContext } from './EditCosts';
import { useParams } from 'react-router-dom';
import { getPeriod } from '../../../../components/Logic/getPeriod';
import { getIdsForMutation } from './utils';
import './editCosts.scss';

import {
  EstimateDrawer,
  fmtPercent,
  fmtPeriod,
  fmtPrice,
  getSettedPeriod,
  handleMutationResult,
  nullFormatterPrice,
  toNumber,
} from './utils';
import { FormItemInput, SaveButton } from '../../../../components/Form/FormEdit';
import { FormItemPercentValue } from './FormItemInputDepend';
import { getConstructionSideCodeByArgs } from '../../../../components/Logic/constructionSideCode';
import moment from 'moment';
import { EDIT_ESTIMATE_ITEMS } from '../q_mutations';
import { AgencyCommissionInputs } from '../../../../components/Wigets/AgencyCommission';

export const EditCostsRtsReservation = () => {
  const [form] = Form.useForm();
  const [editEstimateItems] = useMutation(EDIT_ESTIMATE_ITEMS);
  const [confirmLoading, setConfirmLoading] = useState(false);
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
      setFormValuesRtsReservation(form, editingItem);
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
  // const { id, appId } = useParams();

  const getDrawerTitle = (title, editingItem) => {
    if (!editingItem || Object.keys(editingItem).length === 0)
      return title;
    return `${title} – ${editingItem?.code}`;
  };

  return (
    <EstimateDrawer openModal={openModal} setOpenModal={setOpenModal} title={getDrawerTitle(title, editingItem)}>
      <Form
        layout="inline"
        style={{ flexDirection: 'row' }}
        onFinish={(values) => {
          let [ids, isAll] = getIdsForMutation(isEditing, isPackage, selectedRows, blocki, editingItem);

          if (isEditing || isPackage) {
            handleMutationResult(
              editEstimateItems({
                variables: {
                  rtsReservations: getMutationDataRtsReservation(values),
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
            // handleMutationResult(createAddCosts({
            //   variables: {
            //     input: getMutationDataRtsAdditional(values, editingItem, isEditing, id, appId),
            //   },
            // }), form, setOpenModal, setConfirmLoading, refetch, isEditing);
          }
        }}
        form={form}
      >
        <FormInputsRtsReservation form={form} confirmLoading={confirmLoading} actionTitle={actionTitle}/>
      </Form>
    </EstimateDrawer>
  );
};

const setFormValuesRtsReservation = (form, editingItem) => {
  if (Object.keys(editingItem).length === 0)
    return;
  form.setFieldsValue({
    rentByPriceSetted: editingItem?.setted?.rentByPriceSetted,
    discountPricePercentSetted: editingItem?.setted?.discountPricePercentSetted,
    rentToClientSetted: editingItem?.setted?.rentToClientSetted,
    discountToClientPercentSetted: editingItem?.setted?.discountToClientPercentSetted,
    rentToClientAfterDiscountSetted: editingItem?.setted?.rentToClientAfterDiscountSetted,
    mountingSetted: editingItem?.setted?.mountingSetted,
    printingSetted: editingItem?.setted?.printingSetted,
    additionalSetted: editingItem?.setted?.additionalSetted,
    nalogSetted: editingItem?.setted?.nalogSetted,
    discountNalogPercentSetted: editingItem?.setted?.discountNalogPercentSetted,
    nalogAfterDiscountSetted: editingItem?.setted?.nalogAfterDiscountSetted,
    rentByPriceAfterDiscountSetted: editingItem?.setted?.rentByPriceAfterDiscountSetted,

    agPercent: editingItem.agencyCommission?.percent,
    agSumm: editingItem.agencyCommission?.value,
    toRent: editingItem.agencyCommission?.toRent,
    toNalog: editingItem.agencyCommission?.toNalog,
    toPrint: editingItem.agencyCommission?.toPrint,
    toMount: editingItem.agencyCommission?.toMount,
    toAdditional: editingItem.agencyCommission?.toAdditional,

    branding: editingItem?.brandingBool,
    period: getPeriod(editingItem?.period),
  });
};

export const getEstimateReservations = (data, sort = '', period = '') => {
  let modifiedData = data.reservations.map((r) => {
    return {
      key: r.reservation.id,
      code: getConstructionSideCodeByArgs(
        r.reservation.constructionSide.postcodeTitle,
        r.reservation.constructionSide.numInDistrict,
        r.reservation.constructionSide.formatCode,
        r.reservation.constructionSide.sideCode,
        r.reservation.constructionSide.advSideCode,
      ),
      city: r.reservation.constructionSide.cityTitle || '',
      address: r.reservation.constructionSide.addressTitle || '',
      format: r.reservation.constructionSide.formatTitle || '',
      side: r.reservation.constructionSide.sideTitle || '',
      period: fmtPeriod(r.reservation.dateFrom, r.reservation.dateTo),
      brandingBool: r.reservation.branding,
      branding: r.reservation.branding ? 'Да' : 'Нет',
      rentByPrice: fmtPrice(r.rentByPriceCalculated),
      discountByPrice: fmtPercent(r.discountPricePercentSelected),
      rentOnClient: fmtPrice(r.valueRentToClientSelected),
      discountOnClient: fmtPercent(r.discountClientPercentSelected),
      rentAfterDiscount: fmtPrice(r.valueRentToClientAfterDiscountSelected),
      tax: fmtPrice(r.additionalStaticNalog),
      discountOnTax: fmtPercent(r.additionalStaticNalogDiscountPercentSelected),
      taxAfterDiscount: fmtPrice(r.additionalStaticNalogValueAfterDiscount),
      vat: '',
      mount: fmtPrice(r.additionalStaticMounting),
      print: fmtPrice(r.additionalStaticPrinting),
      additional: fmtPrice(r.additionalStaticAdditional),
      sum: fmtPrice(r.itogSummary),
      percentAK: fmtPercent(r.agencyCommissionPercentSelected),
      sumAK: fmtPrice(r.itogAgencyCommission),
      sumWithoutAK: fmtPrice(r.itogSummaryWithoutAgencyCommission),
      agencyCommission: r.agencyCommission,
      setted: {
        rentByPriceSetted: r.rentByPriceSetted,
        discountPricePercentSetted: r.discountPricePercentSetted,
        rentByPriceAfterDiscountSetted: r.rentByPriceAfterDiscountSetted,
        rentToClientSetted: r.rentToClientSetted,
        discountToClientPercentSetted: r.discountToClientPercentSetted,
        rentToClientAfterDiscountSetted: r.rentToClientAfterDiscountSetted,
        mountingSetted: r.mountingSetted,
        printingSetted: r.printingSetted,
        additionalSetted: r.additionalSetted,
        nalogSetted: r.nalogSetted,
        discountNalogPercentSetted: r.discountNalogPercentSetted,
        nalogAfterDiscountSetted: r.nalogAfterDiscountSetted,
      },
      block: 0,
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
      modifiedData = modifiedData.sort((a, b) => {
        const START = moment(a.period.split(' - ')[0], 'DD.MM.YYYY');
        const END = moment(a.period.split(' - ')[1], 'DD.MM.YYYY');
        const START2 = moment(b.period.split(' - ')[0], 'DD.MM.YYYY');
        const END2 = moment(b.period.split(' - ')[1], 'DD.MM.YYYY');
        const duration = moment.duration(END.diff(START));
        const duration2 = moment.duration(END2.diff(START2));
        return duration._milliseconds - duration2._milliseconds;
      });
      break;

    case 'decrease':
      modifiedData = modifiedData.sort((a, b) => {
        const START = moment(a.period.split(' - ')[0], 'DD.MM.YYYY');
        const END = moment(a.period.split(' - ')[1], 'DD.MM.YYYY');
        const START2 = moment(b.period.split(' - ')[0], 'DD.MM.YYYY');
        const END2 = moment(b.period.split(' - ')[1], 'DD.MM.YYYY');
        const duration = moment.duration(END.diff(START));
        const duration2 = moment.duration(END2.diff(START2));
        return duration2._milliseconds - duration._milliseconds;
      });
      break;

    default:
      break;
  }

  return modifiedData;
};

const getMutationDataRtsReservation = (values) => {
  const [start, end] = getSettedPeriod(values);
  let agencyCommission = {
    value: toNumber(values.agSumm),
    percent: toNumber(values.agPercent),
    toRent: values.toRent,
    toNalog: values.toNalog,
    toPrint: values.toPrint,
    toMount: values.toMount,
    toAdditional: values.toAdditional,
  };
  console.log(agencyCommission);

  const disallowed = [
    'period',
    'agSumm',
    'agPercent',
    'toRent',
    'toNalog',
    'toPrint',
    'toMount',
    'toAdditional',
    'branding',
  ];

  const filtered = Object.keys(values)
    .filter((key) => !disallowed.includes(key))
    .reduce((obj, key) => {
      obj[key] = toNumber(values[key]);
      return obj;
    }, {});

  let input = {
    dateFrom: start,
    dateTo: end,
    branding: values.branding,
    agencyCommission: agencyCommission,
    ...filtered,
  };
  // console.log(input);
  return input;
};

const FormInputsRtsReservation = ({ form, confirmLoading, actionTitle }) => {
  const needBranding = (
    <div className="need-branding">
      Включен
      <br/>
      брендинг
    </div>
  );
  return (
    <div className={'from-input-reservation-grid'}>
      <FormItemInput name="rentToClientSetted" label="Аренда на клиента" formatter={nullFormatterPrice}/>
      <FormItemPercentValue
        name1="discountToClientPercentSetted"
        label1="Скидка на клиента"
        name2="rentToClientAfterDiscountSetted"
        label2="Аренда на клиента со скидкой"
      />
      <FormItemInput name="mountingSetted" label="Монтаж" formatter={nullFormatterPrice}/>
      <FormItemInput name="printingSetted" label="Печать" formatter={nullFormatterPrice}/>
      <FormItemInput name="additionalSetted" label="Доп. расходы." formatter={nullFormatterPrice}/>
      <FormItemInput name="nalogSetted" label="Налог" formatter={nullFormatterPrice}/>
      <FormItemPercentValue
        name1="discountNalogPercentSetted"
        label1="Скидка на налог"
        name2="nalogAfterDiscountSetted"
        label2="Налог после скидки"
      />
      <AgencyCommissionInputs
        needBranding={true}
        needBrandingLabel={needBranding}
        partnerAlignTop={true}
        partnerAlignDefault={false}
      />
      <Form.Item className="editForm-item save-button">
        <SaveButton loading={confirmLoading} actionTitle={actionTitle}/>
      </Form.Item>
    </div>
  );
};
