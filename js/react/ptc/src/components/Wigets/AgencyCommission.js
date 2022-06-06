import { Checkbox, Divider } from 'antd';
import styled from 'styled-components';
import { FormItem } from '../Form/FormItem';
import React from 'react';
import { FormItemPercentValue } from '../../containers/Sales/Estimate/utils/FormItemInputDepend';
import { DebouncedSelectPartner } from '../CustomDebouncedSelect/selects/DebouncedSelectPartner';

export const AgencyCommissionInputs = ({
  needBranding = false,
  needNonRts = false,
  needBrandingLabel = undefined,
  StyledAgencyCommissionDivComponent = undefined,
  partnerAlignDefault = true,
  partnerAlignTop = false,
  preloadedAkAgent = undefined,
  componentIsMounted = undefined,
}) => {
  if (StyledAgencyCommissionDivComponent === undefined)
    StyledAgencyCommissionDivComponent = StyledAgencyCommissionDiv;
  return (
    <>
      <FormItemPercentValue
        name1="agPercent"
        label1="Процент АК"
        className1="ak-percent"
        name2="agSumm"
        label2="Сумма АК"
      />
      <StyledAgencyCommissionDivComponent className="ak-distribute-0">
        <div className={'ak-distribute'}>
          <Divider>АК распространяется на </Divider>
          <div className={'ak-distribute-2'}>
            <FormItem
              name="toRent"
              label="Аренду"
              initialValue={true}
              required={false}
              labelAlign="right"
              valuePropName="checked"
            >
              <Checkbox />
            </FormItem>
            <FormItem
              name="toNalog"
              label="Налог"
              initialValue={false}
              required={false}
              labelAlign="center"
              valuePropName="checked"
            >
              <Checkbox />
            </FormItem>
            <FormItem
              name="toPrint"
              label="Печать"
              initialValue={false}
              required={false}
              labelAlign="center"
              valuePropName="checked"
            >
              <Checkbox />
            </FormItem>
            <FormItem
              name="toMount"
              label="Монтаж"
              initialValue={false}
              required={false}
              labelAlign="center"
              valuePropName="checked"
            >
              <Checkbox />
            </FormItem>
            <FormItem
              name="toAdditional"
              label="Доп. расходы"
              initialValue={false}
              required={false}
              valuePropName="checked"
            >
              <Checkbox />
            </FormItem>
            {needNonRts && (
              <FormItem name="toNonrts" label="НОН РТС" initialValue={false} required={false} valuePropName="checked">
                <Checkbox />
              </FormItem>
            )}
          </div>
        </div>
        {needBranding && (
          <StyledBrandingDiv>
            <FormItem
              name="branding"
              label={needBrandingLabel}
              initialValue={false}
              required={false}
              className="branding"
              valuePropName="checked"
            >
              <Checkbox />
            </FormItem>
          </StyledBrandingDiv>
        )}
      </StyledAgencyCommissionDivComponent>
      <DebouncedSelectPartner
        className="ak-select-partner"
        label="Получатель агентской комиссии"
        dropdownAlignDefault={partnerAlignDefault}
        dropdownAlignTop={partnerAlignTop}
        name="akContragent"
        required={false}
        preloadedKeys={preloadedAkAgent}
        componentIsMounted={componentIsMounted}
      />
    </>
  );
};

const StyledBrandingDiv = styled.div`
  display: flex;
  justify-content: left;
`;

const StyledAgencyCommissionDiv = styled.div`
  grid-column: span 1;
  width: 100%;
  display: grid;
  grid-template-columns: 2fr 1fr;
  gap: 30px;
  .ak-distribute {
    .ant-divider.ant-divider-horizontal {
      margin-top: -1rem;
      margin-bottom: 0.6rem;
    }
    .ant-divider.ant-divider-horizontal > span {
      font-weight: 700;
      font-size: 14px;
    }
    .ak-distribute-2 {
      width: 100%;
      display: grid;
      grid-template-columns: repeat(5, 1fr);
      gap: 30px;
      text-align: center;
      justify-content: center;
      .ant-col:first-child {
        display: flex;
        justify-content: center;
        label {
          display: block;
          span {
            display: block;
          }
        }
      }
      label {
        margin-bottom: -0.3rem;
      }
      .ant-form-item-label > label > span {
        font-weight: 500 !important;
      }
    }
  }
`;
