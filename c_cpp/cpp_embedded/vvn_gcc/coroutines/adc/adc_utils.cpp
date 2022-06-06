/*
 * adc_utils.cpp
 *
 * Created: 18.03.2022 15:23:46
 *  Author: artem
 */


#include <avr/sleep.h>
#include "../../arduino/Arduino.h"
#include "../../src/init_adc.h"
#include "adc_utils.h"

namespace adc {


uint16_t rawAnalogReadWithSleep()
{
    // Генерировать прерывание когда преобразование будет завершено
    ADCSRA |= _BV(ADIE);

    uint8_t previous_timer_control = TCCR0;

    // Отключить таймер
    TCCR0 &= ~(_BV(CS00) | _BV(CS01) | _BV(CS02));

    // Включить спящий режим подавления шума
    set_sleep_mode(SLEEP_MODE_ADC);
    sleep_enable();

    // Запустить преобразование АЦП
    ADCSRA |= _BV(ADSC);

    // Любое прерывание будет пробуждать процессор, включая имеющиеся прерывания миллисекунд
    // Цикл, пока преобразование не будет завершено
    do {
        // Следующая строка кода важна только для второго и последующих проходов.
        // Для первого прохода она не имеет эффекта.
        // Гарантировать, что прерывания включены перед входом в сон
        __asm__ __volatile__("sei");

        // Спать. (ДОЛЖНО вызываться сразу после sei)
        sleep_cpu();
        // Проверка состояния преобразования должна выполняться с отключенными прерываниями, чтобы избежать состояния гонки.
        // Отключение прерываний, чтобы указанные ниже действия выполнялись без прерывания.
        __asm__ __volatile__("cli");
    }
    // Преобразование завершено? Если нет, продолжать цикл
    while (((ADCSRA & _BV(ADSC)) != 0));


    uint16_t result = ADCL;
    result |= ((ADCH & (_BV(0) | _BV(1))) << 8);

    // Больше не спать
    sleep_disable();

    // Включить таймер согласно дефолтным настройкам Arduino
    TCCR0 |= previous_timer_control & (_BV(CS00) | _BV(CS01) | _BV(CS02));

    // Включить прерывания
    __asm__ __volatile__("sei");

    // Ядро Arduino не ожидает прерывания после завершения преобразования, поэтому выключить их
    ADCSRA &= ~_BV(ADIE);

    // Вернуть результат преобразования
    return result;
}


/// Установить измеряемый источник сигнала
// void set_adc_measuerment_channel(adc::AdcChannel cannel)
// {
//     switch (cannel) {
//     case adc::AdcChannel::VISCOSITY: // ADC7 - канал 7 АЦП
//         // Установить канал измерения вязкости
//         ADMUX                                         = ADC_MUX_SELECT_VISCOSITY;
//         adc::ADC_measurement_state.waited_adc_channel = adc::AdcChannel::VISCOSITY;
//         break;
//     case adc::AdcChannel::DENSITY: // ADC5 - канал 5 АЦП
//         ADMUX                                         = ADC_MUX_SELECT_DENCITY;
//         adc::ADC_measurement_state.waited_adc_channel = adc::AdcChannel::DENSITY;
//         break;
//     case adc::AdcChannel::TEMPERATURE: // ADC6 - канал 6 АЦП
//         ADMUX                                         = ADC_MUX_SELECT_TEMPERATURE;
//         adc::ADC_measurement_state.waited_adc_channel = adc::AdcChannel::TEMPERATURE;
//         break;
//     default:
//         break;
//     }
// }


// int16_t to_percent(int adc_input_num, int16_t zero_voltage_percent_val) {
//     double voltage_to_percent = round(voltage[adc_input_num] / voltage_to_percent_coeff);
//     int voltage_to_percent_result = int(voltage_to_percent) - zero_voltage_percent_val;
//     if (voltage_to_percent_result > INT16_MAX) {
//         voltage_to_percent_result = INT16_MAX;
//     }
//     else if (voltage_to_percent_result < INT16_MIN) {
//         voltage_to_percent_result = INT16_MIN;
//     }
//     return int16_t(voltage_to_percent_result);
// }


// int16_t read_temperature_stage2()
// {
//     AdcResult result;
//     double temp;
//     int result_i;
// 
//     result   = adc_measured_voltage_interrupt_results[ADC_INT_RESULT_TEMPERATURE];
//     temp     = result;
//     temp     = (temp - 28) * 0.942028985 - 272.9710144927536;
//     result_i = int(round(temp));
//     if (result_i > INT16_MAX)
//         result_i = INT16_MAX;
//     else if (result_i < INT16_MIN)
//         result_i = INT16_MIN;
//     return static_cast<int16_t>(result_i);
// }
// 
// 
// double measure_adc_stage2(int cannel)
// {
//     AdcResult result;
//     double val;
//     result = adc_measured_voltage_interrupt_results[cannel];
//     val    = result;
//     return (val / 1024.0) * vcc;
// }
// 
// 
// // Чтение значения напряжения Internal Vref
// long read_vcc_stage2()
// {
//     AdcResult result;
//     result = adc_measured_voltage_interrupt_results[ADC_INT_RESULT_VCC];
//     result = 1125300L / result; // Back-calculate AVcc in mV
//     return result;
// }
// 
// 
// double measure_corrected_voltage(int cannel_i, int cannel)
// {
//     double x = measure_adc_stage2(cannel); //- zero_voltage[cannel_i];
//     if (x < 0)
//         x = 0.0;
//     return x;
// }

} // namespace adc
