import { message, Modal } from 'antd';

const DeleteModal = (record, block, refetch, deleteReservation) => {
  const { confirm } = Modal;

  return confirm({
    title: 'Вы уверены что хотите удалить?',
    centered: true,
    onOk() {
      switch (block) {
        case 0:
          deleteReservation({
            variables: {
              id: record.id,
            },
          })
            .then((val) => {
              if (val.data.deleteReservationProject.found) {
                message.success('Успешно удалено.');
                refetch();
              }
              else {
                message.error('Что-то пошло не так');
              }
            })
            .catch((err) => message.error(err.toString()));
          break;
        case 1:
          break;
        default:
          return;
      }
    },
    onCancel() {
      console.log('Cancel');
    },
  });
};

export default DeleteModal;
