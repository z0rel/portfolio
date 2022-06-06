/*
    CalmanFilter.cpp

    Created: 25.02.2022 3:42:26
    Author: artem
*/

#include "CalmanFilter.h"
#include "../freertos/portable.h"


namespace calman {

void CalmanFilter::init(uint16_t container_size)
{
#if CALMAN_DEBUG_ON_INTERNAL_MEMORY
    this->size            = MEASURED_VALUES_SIZE;
#else
    this->previous_values = reinterpret_cast<uint16_t *>(pvPortMalloc(container_size * sizeof(uint16_t)));
    this->size            = container_size;
#endif

    for (uint16_t i = 0; i < this->size; ++i) {
        this->previous_values[i] = 0;
    }
}


void CalmanFilter::add(uint16_t new_value)
{
    if (this->count < this->size) {
        this->last                        = this->count;
        this->previous_values[this->last] = new_value;
        ++this->count;
    }
    else {
        if ((++this->first) >= this->size) {
            this->first = 0;
        }
        if ((++this->last) >= this->size) {
            this->last = 0;
        }
        this->previous_values[this->last] = new_value;
    }
    // uint16_t *a = this->previous_values;
    // uint16_t *b = a;
    // ++b;

    // // TODO: оптимизировать на закольцованность
    // for (uint16_t i = 1; i < this->size; ++i) {
    //     *a = *b;
    //     ++a;
    //     ++b;
    // }

    // this->previous_values[this->size - 1] = new_value;
}


uint16_t CalmanFilter::get_k_item(uint16_t k)
{
    // k: 0...count
    // indexes:
    // first=0...last=count-1  ==> if first==0 then  k_out=k
    // last=0,first=1...count-1  ==> if last==0 then  k_out=k+1 if (k+1 < count)====(k != count-1) else k_out=0
    // 0,...last,first,..count-1 ==> if last>0 and last < count-2 then  k_out=k+first if (k+first < count) else k_out=k+first-count
    // 0,...last=count-2,first=count-1 ==> if last==count-2 then k_out=k+first if (k+first < count) else k_out=k+first-count
    uint16_t k_out = k + this->first;
    if (k_out < this->count) {
        return k_out;
    }
    else {
        return k_out - this->count;
    }
}


// TODO: оптимизировать на закольцованность
float CalmanFilter::add_and_filter_value(uint16_t new_value)
{
    // int16_t xest1[MEASURED_VALUES_SIZE]; // Априорная оценка состояния.
    // int16_t xest2[MEASURED_VALUES_SIZE]; // Апостериорная оценка состояния.
    // float P1[MEASURED_VALUES_SIZE];      // Априорная оценка ошибки.
    // float P2[MEASURED_VALUES_SIZE];      // Апостериорная оценка ошибки.
    // float G[MEASURED_VALUES_SIZE];       // Коэффициент усиления фильтра.

    this->add(new_value);

    // Начальные значения апостериорной оценки состояния и апостериорной оценки ошибки:
    // xest2[0] = 0.0;
    float xest2_k_minus_1 = 0.0;
    // P2[0] = 1.0;
    float P2_k_minus_1 = 1.0;

    // Цикл по отсчётам времени.
    for (uint16_t k = 1; k < this->count; ++k) {
        float xest1_k = xest2_k_minus_1 /* xest2[k - 1] */; // Априорная оценка состояния.
        float P1_k    = /* P2[k - 1] */ P2_k_minus_1 + static_cast<float>(CALMAN_PARAM_Q); // Априорная оценка ошибки.

        // После получения нового значения измерения вычисляем апостериорные оценки:

        float G_k                      = P1_k / (P1_k + static_cast<float>(CALMAN_PARAM_R)); // Коэффициент усиления фильтра.
        xest2_k_minus_1 /* xest2[k] */ = xest1_k + G_k * (static_cast<float>(this->previous_values[this->get_k_item(k)]) - xest1_k); // Апостериорная оценка состояния.
        P2_k_minus_1 /* P2[k] */       = (1 - G_k) * P1_k; // Апостериорная оценка ошибки.
    }

    return xest2_k_minus_1 /* xest2[MEASURED_VALUES_SIZE - 1] */;
}


} // namespace calman
