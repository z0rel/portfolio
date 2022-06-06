import React, { createContext } from 'react';

import { EditCostsRtsReservation } from './EditCostsRtsReservation';
import { EditCostsRtsAdditional } from './EditCostsRtsAdditional';
import { EditCostsNonRTS } from './EditCostsNonRTS';

export const EditCostsContext = createContext();


export const EditCosts = ({ openModal, setOpenModal, block, blocki, editingItem, refetch, isEditing=true, isPackage=false,
                            selectedRows, }) => {

  return <EditCostsContext.Provider
    value={{openModal, setOpenModal, block, blocki, editingItem, refetch, isEditing,
      title: isEditing ? 'Редактирование' : (isPackage ? 'Пакетное изменение' : 'Добавление'),
      actionTitle: isEditing || isPackage ? 'Сохранить' : 'Добавить',
      selectedRows, isPackage
    }}
  >
    {block === 'reservations-rts' && <EditCostsRtsReservation/> }
    {block === 'additional-rts' && <EditCostsRtsAdditional/>}
    {block === 'non-rtc' && <EditCostsNonRTS/>}
  </EditCostsContext.Provider>
}

