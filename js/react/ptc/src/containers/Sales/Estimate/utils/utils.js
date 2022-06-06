import React from 'react';
import { Drawer, message } from 'antd';
import styled from 'styled-components';
import { ReactComponent as ExitIcon } from '../../../../img/sales/exitIcon.svg';
import moment from 'moment';
import { estimate } from '../../../../assets/proto_compiled/proto';

const AdditionalRtsCategory = estimate.AdditionalRtsCategory;

export const fmtPercent = (item) => {
  return item !== null && item !== undefined ? (Math.round(Number(item) * 100) / 100).toString() + ' %' : ' 0 %';
  // return item ? (Math.round(item).toString()) + ' %' : ' 0 %';
};

export const fmtPrice = (item) => {
  return item !== null && item !== undefined ? `${(Math.round(Number(item) * 100) / 100).toLocaleString()} тг.` : '';
};

export const fmtPrice0 = (item) => {
  return item !== null && item !== undefined
    ? (Math.round(Number(item) * 100) / 100).toLocaleString() + ' тг.'
    : '0 тг.';
};

export const fmtPriceNum0 = (item) => {
  return item !== null && item !== undefined
    ? (Math.round(Number(item) * 100) / 100).toLocaleString() + ' тг.'
    : '0 тг.';
};

export const fmtPeriod = (startDate, endDate) => {
  return startDate && endDate
    ? new Date(startDate).toLocaleDateString() + ' - ' + new Date(endDate).toLocaleDateString()
    : '';
};

export const agencyComissionDistributed = (ak, cat) => {
  return (
    ak &&
    cat &&
    ((cat === AdditionalRtsCategory.NALOG && ak.toNalog) ||
      (cat === AdditionalRtsCategory.MOUNTING && ak.toMount) ||
      (cat === AdditionalRtsCategory.PRINTING && ak.toPrint) ||
      (cat === AdditionalRtsCategory.RENT && ak.toRent) ||
      (cat === AdditionalRtsCategory.ADDITIONAL && ak.toAdditional))
  );
};

export const capitalize = (s) => (s ? s[0].toUpperCase() + s.substring(1) : s);

export const toPrice = (val) => {
  if (typeof val == 'number')
    return val;
  if (!val)
    return 0;
  return parseFloat(val.replace(/\s/g, '').split(' ')[0]);
};

export const handleMutationResult = (
  promise,
  {
    form = null,
    setOpenModal = null,
    setConfirmLoading = null,
    refetch,
    isEditing = undefined,
    messageOkStr = undefined,
  },
) => {
  if (messageOkStr === undefined)
    messageOkStr = isEditing ? 'Успешно изменено.' : 'Успешно создано.';

  if (promise) {
    promise
      .then(() => {
        refetch();
        setOpenModal && setOpenModal(false);
        form && form.resetFields();
        setConfirmLoading && setConfirmLoading(false);
        message.success(messageOkStr);
      })
      .catch((err) => {
        setConfirmLoading && setConfirmLoading(false);
        setOpenModal && setOpenModal(false);
        message.error('Что-то пошло не так, попробуйте ещё раз.\n' + err.toString());
        console.log(err);
      });
  }
};

export const valueIsNull = (value) => {
  return value === '' || value === null || value === undefined || value === 'null' || Number.isNaN(value);
};

export const nullFormatterPercent = (value) => {
  return valueIsNull(value) ? ' %' : `${value}%`;
};

export const nullFormatterPrice = (value) => {
  return valueIsNull(value) ? ' тг' : `${value} тг`;
};

export const StyledDrawer = styled(Drawer)`
  .ant-drawer-content-wrapper {
    right: 0px !important;
    width: calc(100% - 60px);
  }
  & .ant-drawer-content,
  .ant-drawer-content-wrapper {
    border-top-left-radius: 8px;
    border-top-right-radius: 8px;
  }
  .editBtn {
    width: 100%;
    // max-width: 270px;
    margin-right: 0 !important;
  }
  .editBtn > div {
    display: flex !important;
    justify-content: flex-end;
  }
  .editForm-item {
    display: flex;
    flex-direction: column;
    margin-right: 0 !important;
  }
  .ant-drawer-header {
    button {
      padding-top: 0.7rem;
    }
  }
`;

export const StyledTitle = styled.span`
  color: #003360;
  font-size: 14px;
  text-transform: uppercase;
`;

export const EstimateDrawer = ({ openModal, setOpenModal, title, children, ...props }) => {
  return (
    <StyledDrawer
      height="auto"
      destroyOnClose
      bodyStyle={{
        paddingBottom: 10,
      }}
      title={<StyledTitle>{title}</StyledTitle>}
      placement="bottom"
      closable={true}
      onClose={() => setOpenModal(false)}
      closeIcon={<ExitIcon/>}
      visible={openModal}
      maskStyle={{ backgroundColor: 'transparent' }}
      key={'12qwe'}
      {...props}
    >
      {children}
    </StyledDrawer>
  );
};

export const toNumber = (val) => {
  if (val === null)
    return null;
  if (val === false || val === '')
    return 0;
  if (typeof val === 'number')
    return val;
  return parseFloat(val);
};

export const getSettedPeriod = (values) => {
  return [
    values && values.period && values.period[0] ? moment(values.period[0]).toDate() : null,
    values && values.period && values.period[1] ? moment(values.period[1]).toDate() : null,
  ];
};

export const StyledPFirst = styled.p`
  font-size: 12px;
  color: #656565;
  margin-bottom: 0;
`;

export const StyledPSecond = styled.p`
  font-size: 12px;
  color: #656565;
  margin-bottom: 0;
  margin-top: 15px;
`;

export const getIdsForMutation = (isEditing, isPackage, selectedRows, block, editingItem) => {
  let ids = [];
  if (isEditing)
    ids = [editingItem.key];
  let isAll = false;
  if (isPackage) {
    let b = selectedRows[block];
    console.log(b, selectedRows, block);
    ids = b?.all ? [] : b?.keys;
    isAll = b?.all;
  }
  return [ids, isAll];
};
