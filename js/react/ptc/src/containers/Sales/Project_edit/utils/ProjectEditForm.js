import React from 'react';
import styled from 'styled-components';
import { Form, Input } from 'antd';
import { BlockBody, BlockTitle, Medium, Row } from '../../../../components/Styles/StyledBlocks';
import { StyledFormItem } from '../../../../components/CustomDebouncedSelect/selects/StyledFormItem';
import { DebouncedSelectSalesManagerId } from '../../../../components/CustomDebouncedSelect/selects/DebouncedSelectSalesManagerId';
import { DebouncedSelectBackOfficeManager } from '../../../../components/CustomDebouncedSelect/selects/DebouncedSelectBackOfficeManager';
import { DebouncedSelectBrand } from '../../../../components/CustomDebouncedSelect/selects/DebouncedSelectBrand';
import { DebouncedSelectClient } from '../../../../components/CustomDebouncedSelect/selects/DebouncedSelectClient';
import { DebouncedSelectAgency } from '../../../../components/CustomDebouncedSelect/selects/DebouncedSelectAgency';
import { AgencyCommissionInputs } from '../../../../components/Wigets/AgencyCommission';
import { StyledFormItemAgency } from '../Project_edit';
import './style.scss';

export const ProjectEditForm = ({
  form,
  preloadedBackOfficeManager = undefined,
  preloadedSalesManager = undefined,
  preloadedCreator = undefined,
  preloadedBrand = undefined,
  preloadedClient = undefined,
  preloadedAgency = undefined,
  preloadedAkAgent = undefined,
  componentIsMounted = undefined,
}) => {
  return (
    <Form form={form}>
      <StyledFormWrapper>
        <div style={{ flex: '1 0 60%', margin: '1vw 1vw 1vw 0' }}>
          <Medium>
            <BlockTitle>О Проекте</BlockTitle>
            <BlockBody>
              <div style={{ display: 'flex', paddingBottom: '30px' }}>
                <StyledProjectBasicInfo2>
                  <StyledFormItem name="title" label="Название проекта">
                    <Input size="large" />
                  </StyledFormItem>
                  <DebouncedSelectSalesManagerId
                    name="creator"
                    label="Создатель"
                    preloadedKeys={preloadedCreator}
                    componentIsMounted={componentIsMounted}
                  />
                  <DebouncedSelectBackOfficeManager
                    name="backOfficeManager"
                    preloadedKeys={preloadedBackOfficeManager}
                    componentIsMounted={componentIsMounted}
                  />
                  <DebouncedSelectSalesManagerId
                    name="salesManager"
                    preloadedKeys={preloadedSalesManager}
                    componentIsMounted={componentIsMounted}
                  />
                </StyledProjectBasicInfo2>
                <div style={{ width: '40%', marginLeft: '20px', marginTop: '1.4rem' }}>
                  <StyledFormItem name="comment" label="Комментарий к проекту">
                    <InfoInput.TextArea rows={6} />
                  </StyledFormItem>
                </div>
              </div>
            </BlockBody>
          </Medium>
        </div>

        <div style={{ flex: '1 0 35%', margin: '1vw 0 1vw 1vw' }}>
          <Medium>
            <BlockTitle>Информация о бренде</BlockTitle>
            <BlockBody>
              <Row>
                <DebouncedSelectBrand
                  style={{ width: '100%' }}
                  dropdownAlignDefault
                  showDescription
                  preloadedKeys={preloadedBrand}
                  componentIsMounted={componentIsMounted}
                />
              </Row>
            </BlockBody>
          </Medium>
        </div>
        <div style={{ flex: '1 0 100%', margin: '1vw 1vw 1vw 0px' }}>
          <Medium>
            <BlockTitle>Доп. инфо</BlockTitle>
            <BlockBody>
              <StyledEditAdditionalInfo>
                <DebouncedSelectClient
                  name="client"
                  dropdownAlignDefault
                  formitem={StyledFormItemAgency}
                  preloadedKeys={preloadedClient}
                  componentIsMounted={componentIsMounted}
                />
                <DebouncedSelectAgency
                  name="agency"
                  dropdownAlignDefault
                  formitem={StyledFormItemAgency}
                  preloadedKeys={preloadedAgency}
                  componentIsMounted={componentIsMounted}
                />
                <div className={'edit-additional-ak'}>
                  <AgencyCommissionInputs
                    toNonrts
                    preloadedAkAgent={preloadedAkAgent}
                    componentIsMounted={componentIsMounted}
                  />
                </div>
              </StyledEditAdditionalInfo>
            </BlockBody>
          </Medium>
        </div>
      </StyledFormWrapper>
    </Form>
  );
};

const StyledEditAdditionalInfo = styled.div`
  display: grid;
  grid-template-columns: 1fr 1fr 1fr;
  grid-gap: 40px 20px;
  @media (max-width: 1200px) {
    grid-template-columns: 1fr 1fr;
  }

  & > .edit-additional-ak {
    grid-column-start: 1;
    grid-column-end: 3;

    display: grid;
    justify-content: start;
    grid-template-columns: 1fr 1fr 3fr 2fr;
    grid-gap: 10px 10px;
    align-items: start;
  }

  margin-bottom: 30px;
`;

const InfoInput = styled(Input)`
  font-weight: 600;
  margin-left: auto;
  width: 150px;
`;

const StyledProjectBasicInfo2 = styled.div`
  margin-top: 1.4rem;
  width: 60%;
  display: grid;
  grid-template-columns: 1fr 1fr;
  grid-gap: 10px 30px;
`;

const StyledFormWrapper = styled.div`
  display: flex;
  flex-wrap: wrap;
  margin-bottom: 20px;
`;
