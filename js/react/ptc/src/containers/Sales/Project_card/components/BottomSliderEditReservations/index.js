import React, { useEffect, useRef, useState } from 'react';
import moment from 'moment';
import { Checkbox, Form } from 'antd';
import { ReactComponent as DateIcon } from '../../../../../img/left-bar/filter/date.svg';
import { useMutation } from '@apollo/client';
import { handleMutationResult } from '../../../Estimate/utils/utils';
import dateFormat from 'dateformat';
import '../bottom-slider.scss';
import { UPDATER_RESERVATIONS } from './q_update_reservations';
import { ReservationDatePicker } from './ReservationDatePicker';
import { DebouncedSelectReservationStatus } from '../../../../../components/CustomDebouncedSelect/selects/DebouncedSelectReservationStatus';
import { useWindowSize } from '../../../../../components/Logic/useWindowSize';
import { SaveButton } from '../../../../../components/Form/FormEdit';
import { FormItem } from '../../../../../components/Form/FormItem';
import { SlidingBottomPanel } from '../../../../../components/SlidingBottomPanel/SlidingBottomPanel';

export function EditReservationSlider({ setShowed, onClose, reservation, refetch, projectId, selectedItems }) {
  const [width] = useWindowSize();
  let [form] = Form.useForm();

  const [updateReservations] = useMutation(UPDATER_RESERVATIONS);
  const [confirmLoading, setConfirmLoading] = useState(false);
  const componentIsMounted = useRef(true);

  const onFormFinish = (values) => {
    form.validateFields().then(() => {
      componentIsMounted.current = false;
      setConfirmLoading(true);

      let dateFrom = values.dateFrom ? dateFormat(values.dateFrom, 'yyyy-mm-dd') + 'T00:00:00+00:00' : null;
      let dateTo = values.dateTo ? dateFormat(values.dateTo, 'yyyy-mm-dd') + 'T00:00:00+00:00' : null;
      let idsSelected =
        selectedItems[0] && selectedItems[0].keys && selectedItems[0].keys.length
          ? selectedItems[0].keys
          : [reservation.id];

      let promise = updateReservations({
        variables: {
          dateFrom: dateFrom,
          dateTo: dateTo,
          project: projectId,
          reservationType: values.status,
          branding: values.branding,
          idsSelected: idsSelected,
        },
      });
      handleMutationResult(promise, { form, setOpenModal: setShowed, refetch, isEditing: true });
    });
  };

  useEffect(() => {
    if (reservation) {
      form.setFieldsValue({
        dateFrom: moment(reservation.dateFrom).toDate(),
        dateTo: moment(reservation.dateTo).toDate(),
        status: reservation.reservationTypeId,
        branding: reservation.branding,
      });
    }
  }, [reservation, form]);

  return (
    <SlidingBottomPanel
      title={`Редактирование брони ${reservation.reservation_code}`}
      onClose={onClose}
      classNameSuffix="loca"
      sliderClass="project-card-slider"
    >
      <Form onFinish={onFormFinish} form={form} layout="vertical" requiredMark="optional">
        <div className="period-wrapper">
          <DateIcon className="date-icon"/>
          <FormItem
            required
            name="dateFrom"
            rules={{ message: 'Пожалуйста, введите дату начала бронирования', required: true }}
            label="Дата начала"
          >
            <ReservationDatePicker placeholder={width > 1330 ? 'Дата начала' : 'Начало'}/>
          </FormItem>
        </div>
        <div className="period-wrapper">
          <DateIcon className="date-icon"/>
          <FormItem
            required
            name="dateTo"
            rules={{ message: 'Пожалуйста, введите дату окончания бронирования', required: true }}
            label="Дата окончания"
          >
            <ReservationDatePicker placeholder={width > 1330 ? 'Дата окончания' : 'Окончание'}/>
          </FormItem>
        </div>
        <DebouncedSelectReservationStatus
          isLongPlaceholder={width > 1290}
          formitem={{ antd: true }}
          componentIsMounted={componentIsMounted}
        />
        <FormItem
          name="branding"
          className="reservation-branding"
          valuePropName="checked"
          required={true}
          label="Дополнительно"
        >
          <Checkbox>Брендирование</Checkbox>
        </FormItem>
        <Form.Item className="editForm-item save-button">
          <SaveButton loading={confirmLoading} actionTitle={'Сохранить'}/>
        </Form.Item>
      </Form>
    </SlidingBottomPanel>
  );
}
