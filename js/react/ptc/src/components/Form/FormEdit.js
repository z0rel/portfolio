import React from 'react';
import { Button, InputNumber, Upload } from 'antd';
import styled from 'styled-components';
import { FormItem } from './FormItem';

export const SaveButton = ({ confirmLoading, actionTitle, ...props }) => {
  return (
    <StyledSaveButton type="primary" htmlType="submit" loading={confirmLoading} {...props}>
      {actionTitle}
    </StyledSaveButton>
  );
};

const StyledSaveButton = styled(Button)`
  width: 100%;
  height: 38px;
  margin-top: 40px;
  border-radius: 4px;
  background-color: #2c5de5;
  
  &:hover, &:focus {
    background-color: #2c5de5;
    color: #ffffff;
    border: 1px solid #2c5de5;
  }
`;

export const UploadButton = ({ actionTitle, beforeUpload, onRemove, listType = 'text', maxCount = 1, showUploadList = true, ...props }) => {
  return (
      <Upload
          listType={listType}
          beforeUpload={beforeUpload}
          onRemove={onRemove}
          maxCount={maxCount}
          showUploadList={showUploadList}
      >
        <StyledUploadButton className="upload-button-main" {...props}>
          {actionTitle}
        </StyledUploadButton>
      </Upload>
  );
};

const StyledUploadButton = styled(Button)`
  width: 100%;
  height: 38px;
  margin-top: 40px;
  border-radius: 4px;
  background-color: #FF5800;
  color: #ffffff;
  border: 1px solid #FF5800;
  
  &:hover, &:focus {
    background-color: #FF5800;
    color: #ffffff;
    border: 1px solid #FF5800;
  }
`;

export const FormItemInput = ({ name, label, formatter, required = false, ...props }) => {
  return (
    <FormItem name={name} label={label} initialValue={null} required={required} {...props}>
      <InputNumber size="large" formatter={formatter} />
    </FormItem>
  );
};
