import React, { useContext, useEffect, useState } from 'react';
import { comProjectContext } from './Com_projects';
import { useHistory } from 'react-router-dom';
import { useQuery } from '@apollo/client';
import { Popover } from 'antd';
import { sales } from '../../../assets/proto_compiled/proto';
import citiesIcon from '../../../img/sales/cities.svg';
import { routes } from '../../../routes';
import { column } from '../../../components/Table/utils';
import { base64ToArrayBuffer } from '../../../components/Logic/base64ToArrayBuffer';
import { ErrorComponent } from '../../../components/Logic/ErrorComponent';
import Tablea from '../../../components/Tablea/Tablea_estimate';
import { useFastSearch } from '../../../components/Tablea/logic/fastSearchHook';

import { QUERY_ALL_PROJECTS } from './queries';

const PopoverList = ({ items }) => {
  return (
    <>
      {items.map((item) => (
        <p key={item.id}>{item.title}</p>
      ))}
    </>
  );
};

const TablePopover = ({ values }) => {
  if (!values || !values.length)
    return;

  if (values.length === 1)
    return values[0].title;

  return (
    <Popover placement="bottom" content={<PopoverList items={values}/>}>
      <div
        style={{
          cursor: 'pointer',
        }}
      >
        <img
          src={citiesIcon}
          style={{
            marginLeft: '5px',
            marginRight: '5px',
            marginBottom: '2px',
          }}
          alt="cities"
        />
        {values[0].title}
      </div>
    </Popover>
  );
};

const stubSorter = (a, b) => 0;

const columns = [
  column('Код проекта', 'code', 60, true, stubSorter),
  column('Наименование проекта', 'title', 90, true, stubSorter),
  column('Бренд', 'brandTitle', 80, true, stubSorter),
  column('Дата создания', 'createdAt', 90, true, stubSorter),
  column('Рекламодатель', 'clientTitle', 90, true, stubSorter),
  column('Рекламное агенство', 'agencyTitle', 90, true, stubSorter),
  column('Город', 'cityTitle', 80, true, stubSorter),
  column('Сектор деятельности', 'workingSectorTitle', 100, true, stubSorter),
  column('Менеджер бэк-офиса', 'backOfficeManagerTitle', 90, true, stubSorter),
  column('Менеджер по продажам', 'salesManagerTitle', 90, true, stubSorter),
];

const mapHeightToLimit = (height) => {
  if (height < 909)
    return 10;
  for (let i = 0; i < 20; ++i) {
    if (height < 960 + 50 * i) {
      return i + 11;
    }
  }
  return 20;
};

const PanelDesign = () => {
  const [filter /*setFilter*/ /*constructionsIdSet*/ /*setConstructionsIdSet*/, ] = useContext(comProjectContext);
  const history = useHistory();
  const height = window.innerHeight;

  let data2 = [];
  const [columnsForPopup, setColumnsForPopup] = useState(columns);
  const [pageInfo, setPageInfo] = useState({ limit: undefined, offset: 0 });
  const [orderBy, setOrderBy] = useState({ orderBy: [] });
  const [fastSearchQuery, onChangeFastSearchValue] = useFastSearch('');

  const { loading, error, data, refetch } = useQuery(QUERY_ALL_PROJECTS, {
    variables: {
      ...filter,
      ...pageInfo,
      ...orderBy,
      fastSearch: fastSearchQuery
    },
  });

  useEffect(() => {
    if (pageInfo.limit === undefined && height > 0)
      setPageInfo({ limit: mapHeightToLimit(height), offset: pageInfo.offset });
  }, [pageInfo.limit, height, pageInfo.offset]);

  if (pageInfo.limit === undefined) {
    pageInfo.limit = mapHeightToLimit(height);
  }

  let totalCount = 0;
  if (error) {
    return <ErrorComponent error={error}/>;
  }
  if (loading) {
    // console.log('loading');
  }

  if (data && data?.searchProjectsOptim?.content) {
    let content = data.searchProjectsOptim.content;
    totalCount = data.searchProjectsOptim?.pageInfo?.totalCount;
    let projects = sales.Projects.decode(base64ToArrayBuffer(content));
    data2 = projects.items.map((project) => {
      return {
        key: project.id,
        code: `#${project.code}`,
        brandTitle: project.brandTitle || '',
        createdAt: project?.createdAt.split('T')[0] || '',
        title: project?.title || '',
        clientTitle: project?.clientTitle || '',
        agencyTitle: project?.agencyTitle || '',
        cityTitle: project.cities.length ? (
          <TablePopover values={project.cities}/>
        ) : (
          project.clientCityTitle || project.agencyCityTitle
        ),
        workingSectorTitle: project.workingSector.length ? <TablePopover values={project.workingSector}/> : '',
        backOfficeManagerTitle: `${project.backOfficeManagerFirstName || ''} ${
          project.backOfficeManagerLastName || ''
        }`,
        salesManagerTitle: `${project.salesManagerFirstName || ''} ${project.salesManagerLastName || ''}`,
      };
    });
  }

  return (
    <>
      <div className="outdoor-table-bar">
        <Tablea
          key="reservations-rts"
          defaultPageSize={pageInfo.limit || mapHeightToLimit(height)}
          columns={columnsForPopup}
          setColumns={setColumnsForPopup}
          data={data2}
          select={true}
          edit={true}
          del={false}
          loading={loading}
          enableChoosePeriod={false}
          notheader={false}
          onRow={(record) => {
            return {
              onDoubleClick: () => history.push(routes.sales.project_card.url(record.key)),
            };
          }}
          // openEditModal={openEditModal}
          // setOpenEditModal={setOpenEditModal}
          setOpenEditModal={(bval) => {
          }}
          setEditingItem={(record) => history.push(routes.sales.project_edit.url(record.key))}
          totalCount={totalCount}
          onChangePage={(page, pageSize) => {
            setPageInfo({ limit: pageSize, offset: (page - 1) * pageSize });
          }}
          onChangeOrderBy={(columns) => {
            setOrderBy({ orderBy: columns });
            refetch();
          }}
          onFastSearchSearch={onChangeFastSearchValue}
          // openDeleteModal={openDeleteModal}
          // deleteMutation={deleteReservationFromProject}
          // refetch={refetch}
          // setDeleted={setDeleted}
        />
      </div>
      <style>
        {`.outdoor-table-bar {
            width: 100%;
          }
          .design-info {
            border-radius: 8px;
            border: 1px solid #d3dff0;
            // height: 100%;
            // padding: 1.5%;
            // flex: 0 1 30vw;
            // margin: 0 2vw 0 0;
          }`}
      </style>
    </>
  );
};

export default PanelDesign;
