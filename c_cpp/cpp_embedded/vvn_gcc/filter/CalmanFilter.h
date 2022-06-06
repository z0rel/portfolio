/*
 * CalmanFilter.h
 *
 * Created: 25.02.2022 3:39:33
 *  Author: artem
 */


#ifndef CALMANFILTER_H_
#define CALMANFILTER_H_

#include "stdint.h"

#define CALMAN_DEBUG_ON_INTERNAL_MEMORY 1

// Длина фильтруемой серии
#define MEASURED_VALUES_SIZE 64

#define CALMAN_SQUARE(v) (v) * (v)

/// Дисперсия случайной величины в модели системы.
#define CALMAN_PARAM_Q (0.01)
/// Среднеквадратичная ошибка измерения
#define CALMAN_PARAM_R (CALMAN_SQUARE(0.14))

namespace calman {

class CalmanFilter {
    /// Массив предыдущих значений
#if CALMAN_DEBUG_ON_INTERNAL_MEMORY
    uint16_t previous_values[MEASURED_VALUES_SIZE];
#else
    uint16_t *previous_values = nullptr;
#endif


    // Размер контейнера
    uint16_t size = 0;
    /// Первый элемент контейнера
    uint16_t first = 0;
    /// Последний элемент контейнера
    uint16_t last = 0;

	/// Количество реально добавленных элементов в контейнер
    uint16_t count = 0;

  public:
    /**
        \brief

        \param Q        Дисперсия случайной величины в модели системы.
        \param R1       Среднеквадратичная ошибка измерения до возведения в квадрат.
        \param init_val Значение инициализации элементов массива

    */
    void init(uint16_t container_size);

    /// Добавить элемент и получить отфильтрованное значение
    float add_and_filter_value(uint16_t new_value);

  private:
    /// Добавить элемент к фильтру с кольцевым сдвигом
    void add(uint16_t new_value);
    uint16_t get_k_item(uint16_t k);
	
};


} // namespace calman

#endif /* CALMANFILTER_H_ */
