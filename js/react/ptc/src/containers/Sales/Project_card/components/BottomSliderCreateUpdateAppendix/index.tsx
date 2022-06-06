import React, { useEffect, useState } from 'react';
import moment from 'moment';
import { Form } from 'antd';
import { useMutation } from '@apollo/client';
import { SlidingBottomPanel } from '../../../../../components/SlidingBottomPanel/SlidingBottomPanel';
import { CustomDatePicker, CustomInput } from '../../../../../components';
import { SaveButton } from '../../../../../components/Form/FormEdit';
import { ReactComponent as ManagerIcon } from '../../../../../img/input/owner.svg';
import {
  CREATE_APPENDIX,
  UPDATE_APPENDIX,
} from '../../queries/queriesCreateAppSlider';
import { DROPDOWN_TOP_ALIGN } from '../../../../../components/Form/dropdownPlacements';

import '../bottom-slider.scss';
import { DebouncedSelectSalesManagerId } from '../../../../../components/CustomDebouncedSelect/selects/DebouncedSelectSalesManagerId';
import { DebouncedSelectCreators } from '../../../../../components/CustomDebouncedSelect/selects/DebouncedSelectCreators';

type CreateAppSliderProps = {
  onClose: () => void;
  refetch: () => void;
  projectId: any;
  editItem?: null | { [key: string]: any };
};

function BottomSliderCreateUpdateAppendix({ onClose, projectId, refetch, editItem }: CreateAppSliderProps): React.ReactElement {
  const [createLoading, setCreateLoading] = useState<boolean>(false);
  const [createAppendix] = useMutation(CREATE_APPENDIX);
  const [updateAppendix] = useMutation(UPDATE_APPENDIX);
  const [form] = Form.useForm();
  const onSaveAppendix = () => {
    setCreateLoading(false);
    refetch();
    onClose();
  };
  const onFinishForm = (values: { [key: string]: any }) => {
    const variables: any = {
      ...values,
      project: projectId,
      reservations: [],
    };
    setCreateLoading(true);
    if (editItem) {
      variables.id = editItem.id;
      void updateAppendix({ variables }).then(onSaveAppendix);
    }
    else {
      void createAppendix({ variables }).then(onSaveAppendix);
    }
  };

  useEffect(() => {
    if (editItem) {
      const { signatoryOne, signatoryTwo, signatoryPosition, salesManager, creator, paymentDate } = editItem;
      form.setFieldsValue({
        signatoryOne,
        signatoryTwo,
        signatoryPosition,
        salesManager: salesManager?.id,
        creator: creator?.id,
        paymentDate: paymentDate ? moment(paymentDate) : undefined,
      });
    }
  }, [editItem, form]);

  return (
    <SlidingBottomPanel
      title={editItem ? 'Редактировать приложение' : 'Создать приложение'}
      onClose={() => onClose()}
    >
      <Form form={form} onFinish={onFinishForm} layout="vertical" className="slider-form">
        <div className="item-wrapper">
          <Form.Item
            name="signatoryOne"
            label="Подписант в именительном падеже"
            rules={[{ required: true, message: 'Пожалуйста, заполните.' }]}
          >
            <CustomInput placeholder="Ф.И.О. подписанта"/>
          </Form.Item>
        </div>
        <div className="item-wrapper">
          <Form.Item
            name="signatoryTwo"
            label="Подписант в родительном падеже"
            rules={[{ required: true, message: 'Пожалуйста, заполните.' }]}
          >
            <CustomInput placeholder="Ф.И.О. подписанта"/>
          </Form.Item>
        </div>
        <div className="item-wrapper">
          <DebouncedSelectCreators
            dropdownAlignTop
            // customFormItem={{ antd: true }}
            placeholderSpec={{
              svg: ManagerIcon,
              title: 'Создатель',
              svgMarginTop: '0',
              needSvgInDropdown: true,
              titleMarginLeft: '-.5rem',
            }}
          />
        </div>
        <div className="item-wrapper">
          <DebouncedSelectSalesManagerId
            rules={[{ required: true, message: 'Пожалуйста, выберите менеджера по продажам.' }]}
            dropdownAlignTop
            name="salesManager"
            customFormItem={{ antd: true }}
          />
        </div>
        <div className="item-wrapper">
          <Form.Item
            name="signatoryPosition"
            label="Должность"
            rules={[{ required: true, message: 'Пожалуйста, заполните.' }]}
          >
            <CustomInput placeholder="Должность подписанта"/>
          </Form.Item>
        </div>
        <div className="item-wrapper">
          <Form.Item
            name="paymentDate"
            label="Дата оплаты"
            rules={[{ required: true, message: 'Пожалуйста, заполните.' }]}
          >
            <CustomDatePicker placeholder="Дата оплаты" dropdownAlign={DROPDOWN_TOP_ALIGN}/>
          </Form.Item>
        </div>
        <div className="item-wrapper save-button-wrapper">
          <Form.Item className="editForm-item save-button">
            <SaveButton confirmLoading={createLoading} actionTitle={'Сохранить'} />
          </Form.Item>
        </div>
      </Form>
    </SlidingBottomPanel>
  );
}

export { BottomSliderCreateUpdateAppendix };
