import React, { useContext } from 'react';
import { batchContext } from './BatchPlacement';
import { useQuery, gql } from '@apollo/client';
import { FilterMenu, SearchTitle, FilterText, StyledPanel } from '../../../components/Styles/StyledFilters';
import { Collapse, Checkbox, DatePicker, Form } from 'antd';
import { StyledSelect } from '../../../components/Styles/DesignList/styles';
import { BtnGroup, ResetButton, SubmitButton } from '../../../components/Styles/ButtonStyles';
import anchorIcon from '../../../img/input/anchor.svg';
import cityIcon from '../../../img/input/city.svg';
import districtIcon from '../../../img/input/district.svg';

const { RangePicker } = DatePicker;

const CITY_T = gql`
  {
    searchCity {
      edges {
        node {
          id
          title
        }
      }
    }
  }
`;
const DISTRICT_T = gql`
  {
    searchDistrict {
      edges {
        node {
          id
          title
        }
      }
    }
  }
`;

// const SEARCH_CONSTRUCTION_SIDE_WITH_RESERVATION = gql`
// query {
//   searchPackage {
//     edges {
//       node {
//         title
//         reservationPackages {
//           edges {
//             node {
//               id
//               dateFrom
//               dateTo
//               reservationType {
//                 title
//               }
//               project {
//                 title
//                 salesManager {
//                   id
//                   firstName
//                   lastName
//                 }
//                 backOfficeManager {
//                   id
//                   firstName
//                   lastName
//                 }
//                 brand {
//                   title
//
//                 }
//               }
//             }
//           }
//         }
//       }
//     }
//   }
// }
// `;

const FilterBar = ({ setRefetch }) => {
  const [form] = Form.useForm();
  const setFilter = useContext(batchContext)[1];
  const onFinish = (values) => {
    setFilter(values);
    console.log('values ', values);
  };

  const onReset = () => {
    form.resetFields();
  };
  const city = useQuery(CITY_T).data;
  const district = useQuery(DISTRICT_T).data;

  // const { loading, error, data, refetch } = useQuery(SEARCH_CONSTRUCTION_SIDE_WITH_RESERVATION);

  // useCallback(() => {
  //   setRefetch(refetch);
  // }, [refetch]);

  // console.log('[packagesObj]', data)
  // const dataArr = data ? data.searchPackage.edges.map(item => {
  //   console.log(item)

  //   return item.node.reservationPackages
  // }) : null;
  // console.log('[dataArr]', dataArr)

  // let edgesData;
  // if(dataArr) {
  //   for(let i = 0; i < dataArr.length; i++) {
  //     for(let j = 0; j < dataArr[i].edges.length; j++) {
  //       edgesData = edgesData ? [...edgesData, dataArr[i].edges[j].node.project.salesManager.id] : [dataArr[i].edges[j].node.project.salesManager.id];
  //     }
  //   }
  // }

  // console.log('[edgesData]', edgesData)

  // let salesManagers =  edgesData ? edgesData.filter((item, index) => {
  //   console.log(edgesData.indexOf(item))
  //   return edgesData.indexOf(item) === 0
  // }  ) : null;

  // console.log('[salesManagers]', salesManagers)

  return (
    <FilterMenu
      onKeyDown={(e) => {
        e.key === 'Enter' && alert('Фильтр');
      }}
    >
      <SearchTitle>
        <FilterText>Поиск</FilterText>
      </SearchTitle>
      <Form form={form} onFinish={onFinish}>
        <Collapse expandIconPosition={'right'}>
          <StyledPanel header="По дате" key="1">
            <Form.Item name="date">
              <RangePicker
                placeholder={['Бронь С', 'По']}
                size={'large'}
                format="DD/MM/YYYY"
                style={{ width: '100%' }}
              />
            </Form.Item>
          </StyledPanel>
          <StyledPanel header="Статус брони" key="2">
            <Checkbox defaultChecked name="statusFree">
              <div className="dot-1"></div>
              <span>Свободно</span>
            </Checkbox>
            <br />
            <Checkbox defaultChecked name="statusReserved">
              <div className="dot-2"></div>
              Забронировано
            </Checkbox>
            <br />
            <Checkbox defaultChecked name="statusApproved">
              <div className="dot-3"></div>
              Утверждено
            </Checkbox>
            <br />
            <Checkbox defaultChecked name="statusSaled">
              <div className="dot-4"></div>
              Продано
            </Checkbox>
            <br />
            <Checkbox defaultChecked name="statusUnavailable">
              <div className="dot-4"></div>
              Недоступно
            </Checkbox>
            <br />
            <Checkbox defaultChecked name="statusAll">
              <div className="dot-4"></div>
              Все
            </Checkbox>
          </StyledPanel>
          <StyledPanel header="По городу" key="3">
            <Form.Item name="city">
              <StyledSelect
                showSearch
                placeholder={
                  <>
                    <img src={cityIcon} alt={'Город'} />
                    <span>Город</span>{' '}
                  </>
                }
                size={'large'}
              >
                {city &&
                  city.searchCity.edges.map((item) => (
                    <StyledSelect.Option key={item.node.id} value={item.node.id}>
                      <img src={cityIcon} alt={item.node.title} />
                      <span>{item.node.title}</span>
                    </StyledSelect.Option>
                  ))}
              </StyledSelect>
            </Form.Item>
            <Form.Item name="district">
              <StyledSelect
                placeholder={
                  <>
                    <img src={districtIcon} alt={'Район'} />
                    <span>Район</span>{' '}
                  </>
                }
                size={'large'}
              >
                {district &&
                  district.searchDistrict.edges.map((item) => (
                    <StyledSelect.Option key={item.node.id} value={item.node.id}>
                      <img src={districtIcon} alt={item.node.title} />
                      <span>{item.node.title}</span>
                    </StyledSelect.Option>
                  ))}
              </StyledSelect>
            </Form.Item>
          </StyledPanel>

          <StyledPanel header="По параметрам" key="4">
            <Form.Item name="package_Title">
              <StyledSelect
                placeholder={
                  <>
                    <img src={anchorIcon} alt={'Пакет'} /> <span>Пакет</span>{' '}
                  </>
                }
                size={'large'}
              >
                <StyledSelect.Option value="A1">
                  <img alt="" src={anchorIcon} />
                  <span>A1</span>
                </StyledSelect.Option>
                <StyledSelect.Option value="A2">
                  <img alt="" src={anchorIcon} />
                  <span>A2</span>
                </StyledSelect.Option>
              </StyledSelect>
            </Form.Item>
            <Form.Item name="salesManager_Id">
              <StyledSelect
                placeholder={
                  <>
                    <img alt="" src={anchorIcon} /> <span>Менеджер по продажам</span>{' '}
                  </>
                }
                size={'large'}
              >
                <StyledSelect.Option value="VkN1c3RvbVVzZXJOb2RlOjY=">
                  <img alt="" src={anchorIcon} />
                  <span>Евсеев Филат</span>
                </StyledSelect.Option>
                <StyledSelect.Option value="VkN1c3RvbVVzZXJOb2RlOjE=">
                  <img alt="" src={anchorIcon} />
                  <span>Петр Иванович</span>
                </StyledSelect.Option>
              </StyledSelect>
            </Form.Item>
            <Form.Item name="backofficeManager_Id">
              <StyledSelect
                placeholder={
                  <>
                    <img alt="" src={anchorIcon} /> <span>Менеджер бэк-офиса</span>{' '}
                  </>
                }
                size={'large'}
              >
                <StyledSelect.Option value="VkN1c3RvbVVzZXJOb2RlOjI=">
                  <img alt="" src={anchorIcon} />
                  <span>Кабанов Иоиль</span>
                </StyledSelect.Option>
                <StyledSelect.Option value="VkN1c3RvbVVzZXJOb2RlOjM=">
                  <img alt="" src={anchorIcon} />
                  <span>Терешин Олег</span>
                </StyledSelect.Option>
              </StyledSelect>
            </Form.Item>
            <Form.Item name="brandTitle">
              <StyledSelect
                placeholder={
                  <>
                    <img alt="" src={anchorIcon} /> <span>Бренд</span>{' '}
                  </>
                }
                size={'large'}
              >
                <StyledSelect.Option value="Jysan Invest">
                  <img alt="" src={anchorIcon} />
                  <span>Jysan Invest</span>
                </StyledSelect.Option>
                <StyledSelect.Option value="Forte Bank">
                  <img alt="" src={anchorIcon} />
                  <span>Forte Bank</span>
                </StyledSelect.Option>
              </StyledSelect>
            </Form.Item>
          </StyledPanel>
        </Collapse>
        <BtnGroup>
          <SubmitButton htmlType="submit">Поиск</SubmitButton>
          <ResetButton onClick={onReset}>Очистить</ResetButton>
        </BtnGroup>
      </Form>
      <style>
        {`
        .ant-collapse-content{
           background-color: #f5f7fa !important;
        }
        `}
      </style>
    </FilterMenu>
  );
};

export default FilterBar;
