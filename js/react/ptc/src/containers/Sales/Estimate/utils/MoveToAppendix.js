import React, { useState } from 'react';
import styled from 'styled-components';
import { Form, message } from 'antd';
import { EstimateDrawer, handleMutationResult } from './utils';
import { SaveButton } from '../../../../components/Form/FormEdit';
import { gql, useMutation } from '@apollo/client';
import { DebouncedSelectAppendix } from '../../../../components/CustomDebouncedSelect/selects/DebouncedSelectAppendix';

export const MoveToAppendix = ({
                                 openModal,
                                 setOpenModal,
                                 refetch,
                                 mappedChoosedBlock,
                                 block,
                                 projectId,
                                 selectedRows,
                               }) => {
  const [form] = Form.useForm();
  const [confirmLoading, setConfirmLoading] = useState(false);
  const [moveToAppendix] = useMutation(MOVE_TO_APPENDIX);

  // console.log(selectedRows, block)

  return (
    <StyledEstimateDrawer
      openModal={openModal}
      setOpenModal={setOpenModal}
      title="Добавить выделенные элементы в приложение"
    >
      <Form
        layout="inline"
        style={{ flexDirection: 'row' }}
        onFinish={(values) => {
          let b = selectedRows ? selectedRows[block] : null;
          if (!values.appendix) {
            message.error('Для добавления в приложение необходимо выбрать приложение');
          }
          else if (!b || (!b.all && (!b.keys || b.keys.length === 0)))
            message.error('Для добавления в приложение необходимо выбрать строки таблицы');
          else {
            handleMutationResult(
              moveToAppendix({
                variables: {
                  appendixId: values.appendix,
                  projectId: projectId,
                  estimateSection: mappedChoosedBlock[block],
                  ids: b.all ? undefined : b.keys,
                  isAll: b.all,
                },
              }),
              { setOpenModal, setConfirmLoading, refetch, messageOkStr: 'Добавление в приложение выполнено успешно' },
            );
          }
          setOpenModal(false);
        }}
        form={form}
      >
        <DebouncedSelectAppendix projectId={projectId} /* Приложение */ />
        <Form.Item className="editForm-item save-button">
          <SaveButton loading={confirmLoading} actionTitle="Добавить в приложение"/>
        </Form.Item>
      </Form>
    </StyledEstimateDrawer>
  );
};

const StyledEstimateDrawer = styled(EstimateDrawer)`
  .ant-drawer-content-wrapper {
    right: 0px !important;
    width: calc(100% - 4vw - 482px);
  }
  & .ant-drawer-body form {
    display: grid;
    grid-template-columns: 2fr 1fr 7fr;
    gap: 30px;
    margin-bottom: 1rem;
    @media (max-width: 1800px) {
      grid-template-columns: 2fr 1fr 4fr;
    }
    @media (max-width: 1400px) {
      grid-template-columns: 2fr 1fr 2fr;
    }
    @media (max-width: 1200px) {
      grid-template-columns: 2fr 1fr 1fr;
    }
    @media (max-width: 1050px) {
      grid-template-columns: 2fr 1fr;
    }
  }
`;

const MOVE_TO_APPENDIX = gql`
  mutation AddEstimateItemToAppendix(
    $appendixId: ID!
    $projectId: ID!
    $estimateSection: String!
    $ids: [ID]
    $isAll: Boolean
  ) {
    addEstimateItemToAppendix(
      appendixId: $appendixId
      estimateSection: $estimateSection
      projectId: $projectId
      ids: $ids
      isAll: $isAll
    ) {
      ok
    }
  }
`;
