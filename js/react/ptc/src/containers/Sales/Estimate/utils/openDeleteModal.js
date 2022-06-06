import { Modal } from 'antd';
import { handleMutationResult } from './utils';

export const openDeleteModal = ({record, deleteEstimateItem, refetch, appendixId}) => {
  const { confirm } = Modal;
  confirm({
    title: 'Вы уверены что хотите удалить?',
    centered: true,
    onOk() {

      handleMutationResult(deleteEstimateItem({
        variables: {
          id: record.key,
          appendixId: appendixId
        },
      }), {refetch, messageOkStr: 'Успешно удалено' });
    },
    onCancel() {
      console.log('Cancel');
    },
  });
};
