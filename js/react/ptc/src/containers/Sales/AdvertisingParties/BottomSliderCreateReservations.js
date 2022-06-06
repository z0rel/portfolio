import React, { useContext, useEffect, useRef, useState } from 'react';
import { Checkbox, Form, message } from 'antd';
import {
  ReactComponent as ProjectIcon,
  ReactComponent as ProjectNameInput,
} from '../../../img/sales/projectNameInput.svg';
import { SlidingBottomPanel } from '../../../components/SlidingBottomPanel/SlidingBottomPanel';
import { useMutation, useQuery } from '@apollo/client';
import { adverContext } from './AdvertisingParties';
import { getProjectCodeWithTitle } from '../../../components/Logic/projectCode';
import { DebouncedSelect } from '../../../components/SearchSelect/DebouncedSelect';
import { CREATE_RESERVATION, QUERY_PROJECTS, QUERY_RESERVATION_TYPE } from './q_queries';
import { SaveButton } from '../../../components/Form/FormEdit';
import { FormItem } from '../../../components/Form/FormItem';
import { CustomDateRangePickerLabelled } from '../../../components';
import mapData from './utils/mapData';
import { useHistory, useParams } from 'react-router';
import { useWindowSize } from '../../../components/Logic/useWindowSize';
import getProjectSelectFilter from '../../../components/Logic/getProjectSelectFilter';
import { QUERY_PROJECT_CARD as QUERY_PROJECT_CARD_S } from '../Project_card/queries/queryProjectCard';
import { QUERY_PROJECT_CARD as QUERY_PROJECT_CARD_M } from '../../Installations/Project_card/queries';
import { APPENDIX_ITEM as QUERY_APPENDIX_ITEM } from '../../Base/Documents/Appendix/Appendix';
import { QUERY_APPENDIX_PROTO } from '../Appendix/appendixQuery';
import { QUERY_SALES_ESTIMATE_PROTO } from '../Estimate/q_queries';

import './styles/bottom-slider.scss';
import { routes } from '../../../routes';
import { normalizeProjectId } from '../Project_card/utils/normalizeProjectId';

const SIDES_FORM = {
  1: '-й стороны',
  2: '-х сторон',
  3: '-х сторон',
  4: '-х сторон',
  5: '-и сторон',
  6: '-и сторон',
  7: '-и сторон',
  8: '-и сторон',
  9: '-и сторон',
  0: '-и сторон',
};

const pluralForm = (value, arr, form_10) => {
  if (!value)
    return '';

  const x = value % 100;
  if (x > 9 && x < 20)
    return `${value}${form_10}`;

  return `${value}${arr[value % 10]}`;
};

export function CreateReservationsSlider() {
  const history = useHistory();
  const [width] = useWindowSize();
  let [form] = Form.useForm();

  const componentIsMounted = useRef(true);
  const [resTypes, setResTypes] = useState([]);
  const [selectedSides, setSelectedSides] = useState([]);
  const [confirmLoading, setConfirmLoading] = useState(false);

  const { chartItems, setChartItems, setSliderAddShowed, offset, setPageLoading, searchQuery } = useContext(
    adverContext,
  );

  const getSelected = (items) => {
    let dst = [];
    if (items) {
      for (let item of items) {
        if (item.isSelected) {
          dst.push(item.content);
        }
      }
    }
    return dst;
  };

  useEffect(() => {
    setSelectedSides(getSelected(chartItems));
  }, [setSelectedSides, chartItems]);
  const [createReservation] = useMutation(CREATE_RESERVATION, {
    refetchQueries(mutationResult) {
      let changedProject = mutationResult?.data?.createOrUpdateReservation?.changedProject;
      let result = [];
      if (changedProject) {
        result = [
          { query: QUERY_PROJECT_CARD_S, variables: { id: changedProject } },
          { query: QUERY_PROJECT_CARD_M, variables: { projectId: changedProject } },
        ];
      }
      let changedAppendix = mutationResult?.data?.createOrUpdateReservation?.changedAppendix;
      if (changedAppendix) {
        result = [
          ...result,
          { query: QUERY_APPENDIX_ITEM, variables: { id: changedAppendix } },
          { query: QUERY_APPENDIX_PROTO, variables: { appendixId: changedAppendix } },
        ];
      }
      if (changedProject || changedAppendix) {
        result = [
          ...result,
          {
            query: QUERY_SALES_ESTIMATE_PROTO,
            variables: { projectId: changedProject, appendixId: changedAppendix },
          },
        ];
      }
      return result;
    },
  });

  const loadResTypes = useQuery(QUERY_RESERVATION_TYPE);

  if (loadResTypes.data && !resTypes.length) {
    setResTypes(loadResTypes.data.searchReservationType.edges);
  }

  const onFormFinish = (values) => {
    form.validateFields().then(() => {
      componentIsMounted.current = false;
      setConfirmLoading(true);
      // console.log(new Date(endYear, endMonth, endDay, 0, 0, 0, 0))
      console.log(values);
      createReservation({
        variables: mapFormValueToMutation(values, selectedSides),
      })
        .then((response) => {
          console.log(response);
          let badSides = response?.data?.createOrUpdateReservation?.badSides;
          let createdReservations = response?.data?.createOrUpdateReservation?.createdReservations;

          if (createdReservations?.length > 0) {
            searchQuery
              .refetch({
                offset: offset.offset,
                limit: offset.limit,
              })
              .then((res) => {
                let scheduleChartItems = mapData(res.data.searchAdvertisingSidesOptim.content, history);
                setChartItems(scheduleChartItems);
                setPageLoading(false);
                console.log(res);
              });
          }
          let l = badSides?.length;
          let ok = createdReservations?.length;
          if (l === 0 && ok > 0) {
            message.success(`Бронирования успешно созданы для ${pluralForm(ok, SIDES_FORM, 'сторон')}!`);
          }
          else if (l > 0 && ok > 0) {
            message.success(`Бронирования успешно созданы для ${pluralForm(ok, SIDES_FORM, 'сторон')}!`);
            message.error(`Бронирование ${pluralForm(l, SIDES_FORM, 'сторон')} невозможно для выбранного периода!`, 6);
          }
          else if (l > 0) {
            message.error(`Бронирование ${pluralForm(l, SIDES_FORM, 'сторон')} невозможно для выбранного периода!`, 6);
          }
          else {
            message.error('Бронирования для не созданы!');
          }

          setConfirmLoading(false);
          setSliderAddShowed(false);
        })
        .catch((err) => {
          componentIsMounted.current = true;
          console.log(err);
        });
    });
  };

  const isProjectSelector = window.location.pathname.startsWith(
    routes.sales.project_card_advertising_parties.startPrefix,
  );
  const urlParams = useParams();

  const getProjectFilterValues = (term) => {
    if (isProjectSelector) {
      return { projectId: normalizeProjectId(urlParams?.id) };
    }
    return getProjectSelectFilter(term);
  };

  return (
    <SlidingBottomPanel
      title={'Быстрая бронь'}
      onClose={() => {
        componentIsMounted.current = false;
        setSliderAddShowed(false);
      }}
      classNameSuffix={'loca'}
      sliderClass="advertising-part-slider"
    >
      <Form
        layout="vertical"
        requiredMark="optional"
        form={form}
        onFinish={onFormFinish}
        initialValues={{projectName: isProjectSelector ? normalizeProjectId(urlParams?.id) : undefined}}
      >
        <CustomDateRangePickerLabelled
          className={'period-wrapper'}
          name="period"
          widthWindow={width}
          widthBreak={1590}
        />
        <DebouncedSelect // Название проекта
          className="project-input"
          name="projectName"
          label="Проект"
          dropdownAlignTop
          disabled={isProjectSelector}
          rules={[{ required: true, message: 'Пожалуйста выберите проект.' }]}
          formitem={{ antd: true }}
          query={QUERY_PROJECTS}
          getQueryVariables={getProjectFilterValues}
          placeholderSpec={{
            svg: ProjectIcon,
            title: 'Название проекта',
          }}
          componentIsMounted={componentIsMounted}
          valueSelector={(node) => node.id}
          queryKey="searchProject"
          dataUnpackSpec={{
            unpackForLocalCompare: (a) => getProjectCodeWithTitle(a.createdAt, a.numInYear, a.title),
          }}
        />
        <DebouncedSelect // Статус бронирования
          className="reservation-type"
          name="reservationType"
          rules={[{ required: true, message: 'Пожалуйста, выберите статус бронирования.' }]}
          label="Статус бронирования"
          dropdownAlignTop
          disabled={false}
          formitem={{ antd: true }}
          query={QUERY_RESERVATION_TYPE}
          getQueryVariables={(term) => {
            return { title_Icontains: term };
          }}
          placeholderSpec={{
            svg: ProjectNameInput,
            title: width > 1290 ? 'Статус бронирования' : 'Статус',
          }}
          componentIsMounted={componentIsMounted}
          valueSelector={(node) => node.id}
          queryKey="searchReservationType"
          dataUnpack={(query) => {
            let nodes = query?.searchReservationType?.edges.filter(
              (node) =>
                node?.node.title !== 'Недоступно' && node?.node.title !== 'Свободно' && node?.node.title !== 'Отменено',
            );
            nodes.sort((a, b) => a.node.level - b.node.level || a.node.title.localeCompare(b.node.title));
            return nodes;
          }}
        />
        <FormItem
          name="additional"
          className="reservation-branding"
          valuePropName="checked"
          initialValue={false}
          required={true}
          label="Дополнительно"
        >
          <Checkbox>Брендирование</Checkbox>
        </FormItem>
        <Form.Item className="editForm-item save-button">
          <SaveButton loading={confirmLoading} actionTitle={'Забронировать'}/>
        </Form.Item>
      </Form>
    </SlidingBottomPanel>
  );
}

const mapDate = (date, hours, minutes, seconds) => {
  const day = date.getDate();
  const month = date.getMonth();
  const year = date.getFullYear();
  return new Date(year, month, day, hours, minutes, seconds);
};

const mapFormValueToMutation = (values, side) => {
  const start = mapDate(values.period[0].toDate(), 0, 0, 0);
  const end = mapDate(values.period[1].toDate(), 23, 59, 59);
  console.log(side);
  return {
    dateFrom: start,
    dateTo: end,
    project: values.projectName,
    branding: values.additional,
    reservationType: values.reservationType,
    sidesIds: side,
  };
};
