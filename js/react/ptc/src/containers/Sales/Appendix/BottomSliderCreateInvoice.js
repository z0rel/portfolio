import React, { useState } from 'react';
import moment from 'moment';
import styled from 'styled-components';
import { Checkbox, DatePicker, Form, Input, message } from 'antd';
import { ReactComponent as DateIcon } from '../../../img/left-bar/filter/date.svg';
import { SlidingBottomPanel } from '../../../components/SlidingBottomPanel/SlidingBottomPanel';
import { SliderCellColRaw, SliderRow } from '../../../components/SlidingBottomPanel/PanelComponents';
import { StyledSelect } from '../../../components/Styles/DesignList/styles';
import { gql, useMutation } from '@apollo/client';
import anchorIcon from '../../../img/input/anchor.svg';
import { useHistory } from 'react-router';
import { useWindowSize } from '../../../components/Logic/useWindowSize';
import { FormItem } from '../../../components/Form/FormItem';
import './bottom-slider.scss';
import { SaveButton } from '../../../components/Form/FormEdit';
import { DebouncedSelectPartnerInvoice } from '../../../components/CustomDebouncedSelect/selects/DebouncedSelectPartnerInvoice';
import { routes } from '../../../routes';

// TODO: Сделать подстановку количества броней крансым
// {/*<span*/}
// {/*  style={{*/}
// {/*    color: '#D42D11',*/}
// {/*    fontWeight: 'bold',*/}
// {/*  }}>*/}
// {/*    (24 шт.)*/}
// {/*  </span>*/}

const ReservationSilderCheckboxesFormItem = styled(FormItem)`
  display: flex;
  flex-direction: column;
  max-width: 220px;
  width: 100%;
  min-width: 220px;
  font-size: 14px;
  font-weight: 400;
`;

const ReservationSliderSubmitButton = styled(SaveButton)`
  margin-left: 2rem;
  fontweight: bold;
  min-width: 10rem;
`;

export const QUERY_PARTNER = gql`
  query SearchPartners($title_Icontains: String) {
    searchPartner(title_Icontains: $title_Icontains) {
      edges {
        node {
          id
          title
        }
      }
    }
  }
`;

export const CREATE_INVOICE = gql`
  mutation CreateInvoice(
    $wholeSum: Float
    $paymentLastDate: DateTime
    $customerPaymentMethod: String
    $avr: Boolean
    $partner: ID
    $appendix: ID
  ) {
    createSalesInvoice(
      input: {
        wholeSum: $wholeSum
        paymentLastDate: $paymentLastDate
        customerPaymentMethod: $customerPaymentMethod
        avr: $avr
        partner: $partner
        appendix: $appendix
      }
    ) {
      invoice {
        id
      }
    }
  }
`;

export function CreateInvoiceSlider({ sliderState, dataCount, smetaSummary, appId }) {
  const [width] = useWindowSize();
  let [form] = Form.useForm();
  const [confirmLoading, setConfirmLoading] = useState(false);
  const [createInvoice] = useMutation(CREATE_INVOICE);
  let history = useHistory();

  const onFormFinish = (values) => {
    form
      .validateFields()
      .then(() => {
        setConfirmLoading(true);

        createInvoice({ variables: mapFormValueToMutation(values) })
          .then((response) => {
            console.log(response);

            message.success('Счет успешно выставлен!');

            history.push(routes.sales.invoice.path);
          })
          .catch((err) => {
            setConfirmLoading(false);
            console.log(err);
            message.error('Что-то пошло не так!');
          });
      })
      .catch((err) => {
        setConfirmLoading(false);
        console.log(err);
        message.error('Что-то пошло не так!');
      });
  };

  // TODO mark up cols

  return (
    <SlidingBottomPanel
      title="Выставление счета"
      classNameSuffix="app"
      onClose={sliderState.closeAdd}
      sliderClass="appendix-slider"
    >
      <Form onFinish={onFormFinish} form={form} layout="vertical" requiredMark="optional">
        <SliderRow>
          <SliderCellColRaw>
            <DateIcon className="date-icon"/>
            <FormItem
              name="dateFrom"
              rules={{ message: 'Пожалуйста, введите дату оплаты по счету', required: true }}
              label="Дата оплаты по счету"
              required
            >
              <ReservationDatePicker placeholder={width > 1330 ? 'Дата оплаты по счету' : 'Дата оплаты'}/>
            </FormItem>
          </SliderCellColRaw>
          <SliderCellColRaw>
            <FormItem
              name="paymentMethod"
              rules={{ message: 'Пожалуйста, выберите способ оплаты', required: true }}
              label="Способ оплаты клиентом"
              required
            >
              <StyledSelect
                placeholder={
                  <>
                    <img src={anchorIcon} alt={'Банковский перевод'}/> <span>Банковский перевод</span>{' '}
                  </>
                }
                size={'large'}
              >
                <StyledSelect.Option value={'Наличные'}>
                  <span>Наличные</span>
                </StyledSelect.Option>
                <StyledSelect.Option value={'Бартер'}>
                  <span>Бартер</span>
                </StyledSelect.Option>
                <StyledSelect.Option value={'Банковский перевод'}>
                  <span>Банковский перевод</span>
                </StyledSelect.Option>
              </StyledSelect>
            </FormItem>
          </SliderCellColRaw>
          <SliderCellColRaw>
            <DebouncedSelectPartnerInvoice formitem={FormItem}/>
          </SliderCellColRaw>
          <SliderCellColRaw>
            <div className="slider-info-col">
              {dataCount.info.map((item, index) => {
                return (
                  <div key={index} className="slider-info-inner">
                    <h5>{item.title}:</h5>
                    <span>{item.data}</span>
                  </div>
                );
              })}
            </div>
          </SliderCellColRaw>
          <SliderCellColRaw>
            <div className="slider-count-col">
              {dataCount.count.map((item, index) => {
                return (
                  <div key={index} className="slider-count-inner">
                    <h5>{item.title}:</h5>
                    <span>{item.data}</span>
                  </div>
                );
              })}
            </div>
          </SliderCellColRaw>
          <SliderCellColRaw>
            <div className="slider-summary">
              <h5>Итого:</h5>
              <span>
                {smetaSummary}
                <FormItem name="wholeSum" label="" initialValue={smetaSummary}>
                  <Input type={'hidden'}/>
                </FormItem>
              </span>
              <ReservationSilderCheckboxesFormItem name="avrForm" valuePropName="checked" label="">
                <Checkbox>Выставление АВР</Checkbox>
              </ReservationSilderCheckboxesFormItem>
            </div>
            <FormItem name="appendixId" label="" initialValue={appId}>
              <Input type={'hidden'}/>
            </FormItem>
            <ReservationSliderSubmitButton loading={confirmLoading} actionTitle={'Выставить счет'}/>
          </SliderCellColRaw>
        </SliderRow>
      </Form>
    </SlidingBottomPanel>
  );
}

const DROPDOWN_TOP_ALIGN = {
  points: ['bl', 'tl'],
  offset: [0, -4],
  overflow: {
    adjustX: 0,
    adjustY: 1,
  },
};

const ReservationDatePicker = ({ value, ...props }) => {
  return (
    <DatePicker
      className="date-picker"
      size={'large'}
      style={{ width: '100%' }}
      value={value ? moment(value) : null}
      dropdownAlign={DROPDOWN_TOP_ALIGN}
      format="DD.MM.YYYY"
      {...props}
    />
  );
};

const mapDate = (date, hours, minutes, seconds) => {
  const day = date.getDate();
  const month = date.getMonth();
  const year = date.getFullYear();
  return new Date(year, month, day, hours, minutes, seconds);
};

const mapFormValueToMutation = (values) => {
  const wholeSum = values.wholeSum.replace(' тг.', '').replace(' ', '');
  const dateFrom = mapDate(values.dateFrom.toDate(), 0, 0, 0);
  const avrForm = values.avrForm ?? false;

  return {
    wholeSum: parseFloat(wholeSum),
    paymentLastDate: dateFrom,
    customerPaymentMethod: values.paymentMethod,
    avr: avrForm,
    partner: values.partnerId,
    appendix: values.appendixId,
  };
};
