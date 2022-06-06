/*
 * adc_utils.h
 *
 * Created: 18.03.2022 15:34:01
 *  Author: artem
 */


#ifndef ADC_UTILS_H_
#define ADC_UTILS_H_

#include <stdint.h>

namespace adc {

enum class AdcChannel : uint8_t { VISCOSITY = 0, DENSITY = 1, TEMPERATURE = 2 };

class ADC_measurement {
  public:
    /// Ожидание завершения измерения в обработчике прерывания
    uint16_t measurementResults[3];

    /// Ожидание завершения измерения в обработчике прерывания
    volatile bool adc_wait_isr = false;

    /// Тип ожидаемого измерения
    volatile AdcChannel waited_adc_channel;
};


/// Текущее состояние последнего измерения
extern ADC_measurement ADC_measurement_state;


/// Установить измеряемый канал (температура, вязкость, плотность)
void set_adc_measuerment_channel(adc::AdcChannel cannel);


// Прочитать текущее значение АЦП в режиме шумоподавления через сон
uint16_t rawAnalogReadWithSleep();


} // namespace adc


#endif /* ADC_UTILS_H_ */
