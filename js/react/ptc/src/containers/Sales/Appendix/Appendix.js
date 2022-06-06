import React from 'react';
import { HeaderTitleWrapper, HeaderWrapper, LeftBar, StyledButton } from '../../../components/Styles/DesignList/styles';
import { PanelAppendix } from './PanelAppendix';
import BreadCrumbs from '../../../components/BreadCrumbs/BreadCrumbs';
import { TitleLogo } from '../../../components/Styles/ComponentsStyles';
import { JobTitle } from '../../../components/Styles/StyledBlocks';
import { ButtonGroup } from '../../../components/Styles/ButtonStyles';
import SearchBtn from '../../../components/LeftBar/SearchBtn';
import PackageBtn from '../../../components/LeftBar/PackageBtn';
import BoxBtn from '../../../components/LeftBar/BoxBtn';
import CreateBtn from '../../../components/LeftBar/CreateBtn';
import { useHistory, useParams } from 'react-router';
import styled from 'styled-components';
import { useMutation, useQuery } from '@apollo/client';
import { CreateInvoiceSlider } from './BottomSliderCreateInvoice';
import { SliderState } from '../../../components/SlidingBottomPanel/SliderState';
import { routes } from '../../../routes';
import { ErrorComponent } from '../../../components/Logic/ErrorComponent';
import { GENERATE_APPENDIX_DOCX, QUERY_APPENDIX_PROTO } from './appendixQuery';
// ICONS
import dollarIcon from '../../../img/dollar.svg';
import collapseIcon from '../../../img/collapse-icon.svg';
import personIcon from '../../../img/person.svg';
import paperIcon from '../../../img/paper.svg';

import { mapAppendixData, getSliderCountData } from './mapAppendixData';
import { base64ToArrayBuffer, saveByteArray } from '../../../components/Logic/base64ToArrayBuffer';
import { message } from 'antd';


const Appendix = () => {
  const history = useHistory();
  const { appId } = useParams();
  const sliderState = new SliderState({ name: '', key: '' });

  const { loading, error, data } = useQuery(QUERY_APPENDIX_PROTO, { variables: { appendixId: appId } });
  const [generateDocx] = useMutation(GENERATE_APPENDIX_DOCX);
  if (error)
    return <ErrorComponent error={error}/>;


  let ctx = {appendixData: {}, mappedData: null, sliderCountData: getSliderCountData(null)}
  if (!loading && data) {
    ctx = mapAppendixData(data.searchSalesAddressProgramProto.content);
  }
  const {appendixData, mappedData, sliderCountData} = ctx;
  const projectId = appendixData?.appendix?.project_Id;
  const projectCode = appendixData?.appendix?.project_Code;
  const appendixCode = appendixData?.appendix?.code;

  const links = [
    { id: routes.root.root.path, value: 'Главная' },
    { id: routes.sales.root.path, value: 'Продажи' },
    { id: routes.sales.com_projects.path, value: 'Проекты' },
    { id: projectId ? routes.sales.project_card.url(projectId) : '',
      value: projectCode ? `Проект ${projectCode}` : 'Проект' },
    { id: routes.sales.appendix.url(appId),
      value: appendixCode ? `Приложение ${appendixCode}` : 'Приложение' },
  ];

  let onExportClick = () => {
    generateDocx({variables: {appId}})
      .then((response) => {
        let ab = base64ToArrayBuffer(response.data.generateAppendixDocx.content);
        saveByteArray(`Приложение ${appendixCode}.docx`, ab, 'application/docx');
      })
      .catch(error => {
        message.error('Что-то пошло не так, попробуйте ещё раз.\n' + error.toString());
        console.error(error)
      })
  }


  return (
    !loading && (
      <div style={{ display: 'flex', height: '100%' }}>
        <LeftBar className="left-bar">
          <SearchBtn/>
          <CreateBtn text="Создать новое"/>
          <PackageBtn text="Изменить текущее"/>
          <BoxBtn text="Архив дизайнов"/>
        </LeftBar>
        <div
          style={{
            overflow: 'hidden',
            width: '100%',
            margin: '0 2vw 0 0',
          }}
        >
          <BreadCrumbs links={links} fromRoot={true}/>
          <HeaderWrapper>
            <HeaderTitleWrapper>
              <TitleLogo/>
              <JobTitle>Приложение №{mappedData.code}</JobTitle>
            </HeaderTitleWrapper>
            <ButtonGroup>
              <StyledButton
                backgroundColor="#008556"
                onClick={() => {
                  sliderState.setAddShowed(true);
                }}
              >
                Выставить счет
              </StyledButton>
              <StyledButton backgroundColor="#2C5DE5" onClick={onExportClick}>Выгрузка данных</StyledButton>
              <StyledButton
                backgroundColor="#2C5DE5"
                onClick={() => history.push(routes.sales.appendix_estimate.url(appId))}
              >
                Смета приложения
              </StyledButton>
            </ButtonGroup>
          </HeaderWrapper>
          <div style={{ display: 'flex' }}>
            <InfoList>
              <InfoItem>
                <PanelHead img={collapseIcon} alt="collapse icon" title="Информация о приложении"/>
                <PanelItem title="Дата создания:" value={mappedData.createdDate}/>
                <PanelItem title="Дата начала:" value={mappedData.startedDate}/>
                <PanelItem title="Дата окончания:" value={mappedData.endDate}/>
              </InfoItem>
              <InfoItem>
                <PanelHead img={paperIcon} alt="paper icon" title="Информация о договоре"/>
                <PanelItem title="Номер:" value={mappedData.contractCode}/>
                <PanelItem title="Подписан:" value={mappedData.contractDate}/>
              </InfoItem>
              <InfoItem>
                <PanelHead img={personIcon} alt="person icon" title="Информация о подписанте"/>
                <PanelItem title="ФИО:" value={mappedData.signatoryOne}/>
                <PanelItem title="Должность:" value={mappedData.signatoryPosition}/>
                {/* TODO: вывести должность подписанта */}
              </InfoItem>
              <InfoItem>
                <PanelHead img={dollarIcon} alt="dollar icon" title="Оплата"/>
                <PanelItem title="Срок оплаты:" value={mappedData.paymentDate}/>
                <PanelItem title="Стоимость:" value={mappedData.smetaSummary}/>
              </InfoItem>
            </InfoList>
            <PanelAppendix
              style={{ flex: '0 1 auto' }}
              tableData={mappedData.addressProgramm}
              loading={loading}
              onExportClick={onExportClick}
            />
          </div>
        </div>

        <style>
          {`
          .left-bar {
            margin: 0 2vw 0 0;
          }

        `}
        </style>
        {sliderState.addShowed && (
          <CreateInvoiceSlider
            sliderState={sliderState}
            dataCount={sliderCountData}
            smetaSummary={mappedData.smetaSummary}
            appId={appId}
          />
        )}
      </div>
    )
  );
};

const PanelHead = ({ img, alt, title }) => {
  return (
    <InfoTitle>
      <img src={img} alt={alt}/>
      <span>{title}</span>
    </InfoTitle>
  );
};

const PanelItem = ({ title, value }) => {
  return (
    <InfoLine>
      <span>{title}</span>
      <InfoValue>{value}</InfoValue>
    </InfoLine>
  );
};

export default Appendix;
const InfoList = styled.ul`
  border-radius: 8px;
  border: 1px solid #d3dff0;
  height: 100%;
  padding: 1.5%;
  flex: 0 1 auto;
  margin: 0 2vw 0 0;
  max-width: 370px;
  box-sizing: border-box;
  width: 40vw;
`;
const InfoItem = styled.li`
  margin: 4% 0;
  display: flex;
  flex-direction: column;
  border-bottom: 1px solid #d3dff0;
`;
const InfoTitle = styled.h3`
  font-size: 16px;
  font-weight: 700;
  display: flex;
  align-items: center;
  gap: 12px;
`;
const InfoLine = styled.div`
  margin: 4% 0;
  display: flex;
  justify-content: space-between;
  font-size: 14px;
`;
const InfoValue = styled.span`
  font-weight: 600;
  text-align: right;
`;
