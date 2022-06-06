import { gql } from '@apollo/client';
import { message } from 'antd';

export const UPDATE_MUTATION = gql`
  mutation updateRes($id: ID!, $input: UpdateReservationInput!) {
    updateReservation(id: $id, input: $input) {
      reservation {
        dateFrom
        dateTo
      }
    }
  }
`;

const mapDate = (date, hours, minutes, seconds) => {
  const day = date.getDate();
  const month = date.getMonth();
  const year = date.getFullYear();

  return new Date(year, month, day, hours, minutes, seconds);
};

const updateReservation = (update, items, setPageLoading) => {
  const updateArr = items.map((r) => {
    console.log(mapDate(r.dateFrom), mapDate(r.dateTo));
    return update({
      variables: {
        id: r.id,
        input: {
          dateFrom: mapDate(r.dateFrom, 0,0,0),
          dateTo: mapDate(r.dateTo, 23,59,59),
          agencyCommission: {},
        },
      },
    });
  });
  setPageLoading(true);

  Promise.all(updateArr)
    .then(() => {
      setPageLoading(false);
      message.success('Успешно сохранено.');
    })
    .catch(() => message.error('Что-то пошло не так, попробуйте ещё раз'));
};

export default updateReservation;
