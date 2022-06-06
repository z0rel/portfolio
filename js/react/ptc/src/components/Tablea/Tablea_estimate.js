import { useEffect, useLayoutEffect, useState, useCallback } from 'react';
import PropTypes from 'prop-types';
import { Layout } from 'antd';

import './Tablea.scss';
import { getFrontendSortedData, getSortColumns } from './logic/getSortColumns';
import { CustomPagination } from './components/CustomPagination';
import { TableaEstimateHeaderBar } from './components/HeaderBar';
import { StyledTable } from './components/StyledTable';
import { TableaPageInfo } from './logic/TableaPageInfo';
import { ResizableTitle } from './components/ResizableTitle';
import { doStylingHeader } from './components/stylingHeader';
import { handlerRowSelection } from './logic/handlerRowSelection';
import { getLocalColumns } from './logic/getLocalColumns';

const { Content } = Layout;




export const Tablea = ({
                         columns, // Столбцы для отображения и кнопки-шестеренки
                         setColumns = null, // Установщик новых столбцов для кнопки-шестеренки (вызывается при переходах между табами)
                         data, // Данные для вывода в таблицу
                         onRow, // Обработчик даблклика по строке
                         enableGearButton, // Включить кнопку-шестеренку (true по умолчанию)
                         disableExportButton, // Выключить кнопку экспорта (false пр умолчанию)
                         notheader, // не выводить верхнюю полоску с кнопкой-шестеренкой
                         enableChoosePeriod, // Включить выбор периода в верхней полоске
                         title, // Заголовок внутри верхней полоски
                         chooseTableBtns, // Кнопки (слева) внутри верхней полоски
                         choosedBlock,  // Идентификатор выбранного блока
                         setChoosedBlock, // Установщик нового выбранного блока (нового идентификатора)
                         loading, // Состояние загрузки
                         select, // Нужна ли возможность выбора строк по умолчанию или нет
                         footer, // Футер таблицы antd
                         del = null, // Вывести корзину справа для удаления
                         openDeleteModal, // Обработчик открытия модального окна удаления
                         edit, // Вывести карандаш справа для редактирования
                         setOpenEditModal, // Активатор режима редактирования. При нажатии на карандаж вызывается с значением True
                         setEditingItem, // Обработчик редактирования. При нажатии на карандаш вызывается с значением текущей строки.
                         selectedItems = null, // Объект выбранных элементов (хранит объект для всех блоков, с ключами по choosedBlock)
                         setSelectedItems = null, // Установщик объекта выбранных элементов (хранит объект для всех блоков, с ключами по choosedBlock)
                         defaultPageSize = undefined, // Число элементов в таблице "По умолчанию"
                         paginationRelayApi=false, // Пагинация выполняется через Relay Api или собственную реализацию (при Relay API для сортировки по убыванию устанавливается -, при собственной реализации - _)
                         frontendPagination=false, // Пагинация таблицы только на фронтэнде
                         totalCount, // Значение общей длины выборки (задается при пагинации)
                         onChangePage, // Вызывается при изменении страницы и размера пагинации, если задано
                         onChangeOrderBy, // Вызывается при изменении порядка сортировки столбцов
                         onChangeFilterBy, // Вызывается при изменении фильтра столбцов
                         onFastSearchSearch, // Обработчик клика по кнопке "Найти" - быстрый поиск,
                         onExportClick=undefined // Обработчик клика по кнопке "Экспорт"
                       }) => {

  const computeNewColumns = useCallback(() => getLocalColumns({
    columns,
    edit,
    setOpenEditModal,
    setEditingItem,
    del,
    openDeleteModal,
    choosedBlock,
  }), [columns, openDeleteModal])

  let [columnsFilter, setColumnsFilter] = useState(computeNewColumns());

  // columns changed
  useEffect(() => {

    setColumnsFilter(computeNewColumns());

  }, [columns, openDeleteModal])

  let [pagination, setPagination] = useState({});
  let [selectAll, setSelectAll] = useState({ [choosedBlock]: false });
  let [sorterObj, setSorterObj] = useState(null)
  let [currentPageInfo, setCurrentPageInfo] = useState(new TableaPageInfo(
    0,
    defaultPageSize === undefined ? (window.innerHeight > 1130 ? 15 : 10) : defaultPageSize
    ));

  let selectedRowKeys = [];
  let localData = data;

  if (frontendPagination) {
    localData = getFrontendSortedData(sorterObj, data);
  }

  if (selectAll) {
    selectedRowKeys = localData.map((item) => item.key);
  }

  // reset pagination, when total count changes
  useEffect(() => {

    setPagination(new TableaPageInfo(1, defaultPageSize))

  }, [totalCount])

  useLayoutEffect(() => {
    // Выполнить стилизацию многострочного заголовка
    doStylingHeader()
  });

  let handleResize = (index) => {
    return (e, { size }) => {
      let nextColumns = [...columnsFilter];

      nextColumns[index] = {
        ...nextColumns[index],
        width: size.width,
      };

      const delta = columnsFilter[index].width - size.width;

      nextColumns[index + 1].width += delta;

      setColumnsFilter(nextColumns);
    }
  };

  return (
    <div style={{ width: '100%', overflowX: 'hidden' }}>
      {!notheader && (
        <TableaEstimateHeaderBar
          enableChoosePeriod={enableChoosePeriod}
          title={title}
          chooseTableBtns={chooseTableBtns}
          choosedBlock={choosedBlock}
          setChoosedBlock={(index) => {
            setChoosedBlock(index);
          }}
          enableGearButton={enableGearButton}
          disableExportButton={disableExportButton}
          columnsForPopup={columns}
          setColumnsForPopup={setColumns}
          onFastSearchSearch={onFastSearchSearch}
          onExportClick={onExportClick}
        />
      )}

      <Content>
        <StyledTable
          className={'custom-tablea-antd'}
          onRow={onRow}
          rowSelection={select && handlerRowSelection({
            setSelectedItems,
            selectedItems,
            choosedBlock,
            selectAll,
            setSelectAll,
            selectedRowKeys,
          })}
          loading={loading}
          /*bordered* TODO: сделать Header bordered по макету */
          footer={footer ? () => footer : undefined}
          components={{ header: { cell: ResizableTitle } }}
          columns={columnsFilter.map((col, index) => {
            return {
              ...col,
              onHeaderCell: (column) => {
                return {
                  width: column.width,
                  onResize: handleResize(index),
                };
              },
            };
          })}
          dataSource={currentPageInfo.getSlicedData(localData, pagination, choosedBlock, frontendPagination)}
          scroll={{ x: '100%', y: '100vw' }}
          pagination={false}
          onChange={(pagination, filters, sorter, extra) => {

            if (onChangeOrderBy) {
              onChangeOrderBy(getSortColumns(sorter, paginationRelayApi));
            }
            if (onChangeFilterBy) {
              onChangeFilterBy(filters);
            }
            if (frontendPagination) {
              setSorterObj(sorter)
            }
          }}
        />
        <CustomPagination
          disabled={loading}
          total={totalCount ? totalCount : localData.length}
          current={pagination && pagination[choosedBlock] ? pagination[choosedBlock].page : 1}
          onChange={(page, _pageSize) => {
            setPagination({ ...pagination, [choosedBlock]: new TableaPageInfo(page, _pageSize) });
            if (onChangePage) {
              onChangePage(page, _pageSize);
            }
          }}
          showItems={currentPageInfo.getToShowItems(pagination, choosedBlock, frontendPagination)}
          onChangePageSize={(page, _pageSize) => {
            page = (totalCount ?? localData.length) >= _pageSize ? page : 1;

            setCurrentPageInfo(new TableaPageInfo(page, _pageSize))
            setPagination({ ...pagination, [choosedBlock]: new TableaPageInfo(page, _pageSize) });
            if (onChangePage) {
              onChangePage(page, _pageSize);
            }
          }}
          pageSize={pagination && pagination[choosedBlock] ? pagination[choosedBlock].pageSize : currentPageInfo.pageSize}
        />
      </Content>
    </div>
  );
};

Tablea.propTypes = {
  enableChoosePeriod: PropTypes.bool,
  enableGearButton: PropTypes.bool,
  columns: PropTypes.array,
  onRow: PropTypes.func,
};
Tablea.defaultProps = {
  enableChoosePeriod: false,
  enableGearButton: true,
  columns: [],
  onRow: () => undefined,
};

export default Tablea;



