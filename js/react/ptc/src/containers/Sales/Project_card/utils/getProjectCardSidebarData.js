import collapseIcon from '../../../../img/collapse-icon.svg';

export const getProjectCardSidebarData = (dataItem) => (
  dataItem ? [
    {
      id: 1,
      title: 'О проекте',
      icon: collapseIcon,
      isShowed: true,
      sumBlock: false,
      content: [
        {
          title: 'Код проекта:',
          value: `#${dataItem?.code || ''}`,
        },
        {
          title: 'Менеджер бэк-офиса:',
          value: (dataItem?.backOfficeManager?.firstName || '') + ' ' + (dataItem?.backOfficeManager?.lastName || ''),
        },
        {
          title: 'Менеджер по продажам:',
          value: (dataItem?.salesManager?.firstName || '') + ' ' + (dataItem?.salesManager?.lastName || ''),
        },
      ],
    },
    {
      id: 2,
      title: 'Информация о бренде',
      icon: collapseIcon,
      isShowed: true,
      sumBlock: false,
      content: [
        {
          title: 'Бренд:',
          value: (dataItem?.brand?.title || ''),
        },
        {
          title: dataItem?.brand?.workingSector ? 'Сектор деятельности:' : '',
          value: (`${dataItem?.brand?.workingSector?.description} / ${dataItem?.brand?.workingSector?.title}` || '')
        },
      ],
    },
    {
      id: 3,
      title: 'Доп. инфо',
      icon: collapseIcon,
      isShowed: true,
      sumBlock: false,
      content: [
        {
          title: 'Рекламодатель:',
          value: dataItem?.client?.title || '-',
        },
        {
          title: 'Рекламное агентство:',
          value: dataItem?.agency?.title || '',
        },
        {
          title: dataItem?.agencyCommission ? 'Агентская комиссия:' : '',
          value: dataItem?.agencyCommission
            ? (
              dataItem.agencyCommission?.value
              ? dataItem.agencyCommission?.value
              : dataItem.agencyCommission?.percent
            )
            : '',
        },
      ],
    },
    {
      id: 4,
      title: 'Комментарий к проекту',
      icon: collapseIcon,
      isShowed: true,
      sumBlock: false,
      content: [
        {
          title: dataItem?.comment || '',
        },
      ],
    },
  ]
  : null);
