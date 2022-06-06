import React, { useState, createContext } from 'react';
import { LeftBar, StyledButton, HeaderWrapper, HeaderTitleWrapper } from '../../../components/Styles/DesignList/styles';
import PanelDesign from './PanelInvoice';
import BreadCrumbs from '../../../components/BreadCrumbs/BreadCrumbs';
import { TitleLogo } from '../../../components/Styles/ComponentsStyles';
import { JobTitle } from '../../../components/Styles/StyledBlocks';
import { ButtonGroup } from '../../../components/Styles/ButtonStyles';
import SearchBtn from '../../../components/LeftBar/SearchBtn';
import CreateBtn from '../../../components/LeftBar/CreateBtn';
import PackageBtn from '../../../components/LeftBar/PackageBtn';
import EditBtn from '../../../components/LeftBar/EditBtn';
import BoxBtn from '../../../components/LeftBar/BoxBtn';
import PaperBtn from '../../../components/LeftBar/PaperBtn';
import FilterBar from './FilterBar';

export const invoiceContext = createContext();

const Invoice = () => {
  const [collapsed, setCollapsed] = useState(true);
  const [filter, setFilter] = useState({});
  const links = [
    { id: '', value: 'Главная' },
    { id: 'sales', value: 'Продажи' },
    { id: 'sales/invoice', value: 'Счета' },
  ];

  return (
    <invoiceContext.Provider value={[filter, setFilter]}>
     <div style={{ display: 'flex', height: '100%' }}>
       <div className="flex-margin">
         <LeftBar>
           <SearchBtn onClick={() => setCollapsed(!collapsed)} />
           <CreateBtn text="Добавить бронь" />
           <PackageBtn text="Добавить пакет" />
           <EditBtn text="Перейти в монтажи" />
           <PaperBtn text="Сводка проекта" />
           <BoxBtn text="Архив дизайнов" />
         </LeftBar>
         {collapsed && <FilterBar />}
       </div>

       <div style={{ overflowX: 'hidden', margin: '0 2vw 0 0' }}>
         <BreadCrumbs links={links} />
         <HeaderWrapper>
           <HeaderTitleWrapper>
             <TitleLogo />
             <JobTitle>Счета - CocaCola</JobTitle>
           </HeaderTitleWrapper>
           <ButtonGroup>
             <>
               <StyledButton backgroundColor="#FF5800">Создать отчет</StyledButton>
             </>
           </ButtonGroup>
         </HeaderWrapper>

         <div style={{ display: 'flex' }}>
           <PanelDesign style={{ flex: '0 1 auto' }} />
         </div>
       </div>
       {/* {block === 0 ? null : <FilterBar />} */}
       <style>
         {`
          .flex-margin {
             display: flex;
             margin: 0 2vw 0 0;
           }
           .left-bar {
             margin: 0 2vw 0 0;
           }
         `}
       </style>
     </div>
    </invoiceContext.Provider>
  );
};

export default Invoice;
