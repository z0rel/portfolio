/*
 * adc.cpp
 *
 * Created: 10.03.2022 6:43:23
 *  Author: artem
 */

#include <avr/sleep.h>
#include <stdint.h>

#include "../../arduino/Arduino.h"
#include "../../arduino/HardwareSerial.h"
#include "../../filter/fileter_container.h"
#include "../../src/config_vvn8/config_vvn8.h"
#include "../../src/init_adc.h"
#include "../../src/state/hmi_state.h"

#include "adc_utils.h"

// TODO: сделать фильтрацию калмана корутиной

// TODO: сделать modbus таблицу и modbus корутину
// TODO: сделать журнал, отображаемый в modbus-таблицу
// TODO: сделать оцифровку кнопок - https://www.arduino.cc/en/Tutorial/BuiltInExamples/Debounce
// TODO: сделать виджет ввода
// TODO: доделать hmi интерфейс


// Если мы не предоставляем обработчик прерывания, компилятор/библиотека времени выполнения предоставляет его для нас.
// Обработчик по умолчанию сбрасывает приложение. Это не то, что нам нужно, поэтому нужно предоставить обработчик.
// В обработчике нет кода, потому что прерывание используется только для пробуждения процессора.
ISR(ADC_vect) {}

#define DAC_CODE_FOR_4MA 572
#define DAC_CODE_FOR_4MA_F 572.0
#define DAC_CODE_FOR_20MA 4096
#define DAC_CODE_FOR_20MA_F 4096.0


inline void set_dac_code()
{
    double scaled_dac_code;
    double _t;

    // Масштабировать коды для 4 и 20 миллиампер
    STATIC_LINEAR_INTERPOLATE(4.0, DAC_CODE_FOR_4MA, 20.0, DAC_CODE_FOR_20MA, _t, scaled_dac_code, state::state_output_current.dac_value_current);
    if (scaled_dac_code > DAC_CODE_FOR_20MA_F) {
        scaled_dac_code = DAC_CODE_FOR_20MA_F;
    }
    if (scaled_dac_code < DAC_CODE_FOR_4MA_F) {
        scaled_dac_code = DAC_CODE_FOR_4MA_F;
    }
    int16_t dac_code = static_cast<int16_t>(round(scaled_dac_code));
    if (dac_code < DAC_CODE_FOR_4MA) {
        dac_code = DAC_CODE_FOR_4MA;
    }
    if (dac_code > DAC_CODE_FOR_20MA) {
        dac_code = DAC_CODE_FOR_20MA;
    }

    state::state_output_current.dac_value_code = static_cast<uint16_t>(dac_code);
}


#define ADC_CODE_VISCOSITY_0V_F 25.0
#define ADC_VOLTAGE_VISCOSITY_0_V 0.0
#define ADC_CODE_VISCOSITY_MAX_V_F 1023.0
#define ADC_VOLTAGE_VISCOSITY_MAX_V_F 2.64


#define CONSTRAINT_VAR_BY_ADC_LIMITS(var) \
    if ((var) < 0) {                      \
        (var) = 0;                        \
    }                                     \
    if ((var) > 1023) {                   \
        (var) = 1023;                     \
    }


void measure_viscosity()
{
    ADMUX = ADC_MUX_SELECT_VISCOSITY;

    // Вывести значение сырого кода АЦП для вязкости во входное состояние
    state::state_input_current.viscosity_raw = adc::rawAnalogReadWithSleep();

    double filtered_value = static_cast<double>(calman::calmanContainer.viscosity.add_and_filter_value(state::state_input_current.viscosity_raw));

    // Вывести отфильтрованного кода АЦП для вязкости во входное состояние
    int16_t filtered_viscosity_adc_code = static_cast<int16_t>(round(filtered_value));
    CONSTRAINT_VAR_BY_ADC_LIMITS(filtered_viscosity_adc_code);
    state::state_input_current.viscosity_raw_filtered = static_cast<uint16_t>(filtered_viscosity_adc_code);

    // v input before Operational Amplifier D1:B = -0.1V -> vin = 0.0625411, adc code = 25, calman filtered = 25.0
    // v input before Operational Amplifier D1:B = 2.5V -> vin = 2.5V, adc code = 1000, calman filtered = 1000
    // max adc code = 1023

    // Для вязкости значение АЦП не выравнивается (не юстируется)
    // Преобразовать значение АЦП в напряжение
    // Вывести значение напряжения для вязкости во входное состояние
    double _t;
    STATIC_LINEAR_INTERPOLATE(ADC_CODE_VISCOSITY_0V_F, ADC_VOLTAGE_VISCOSITY_0_V, ADC_CODE_VISCOSITY_MAX_V_F, ADC_VOLTAGE_VISCOSITY_MAX_V_F, _t, state::state_input_current.viscosity_voltage,
                              filtered_value);

    // интерполировать значение АЦП по точкам
    double interpolated_value_of_viscosity = config::config_vvn8.viscosity_interp.calculate(state::state_input_current.viscosity_voltage);

    // Вывести текущую кинематическую вязкость во входное состояние
    state::state_input_current.viscosity_value_kinematik = interpolated_value_of_viscosity;

    // TODO: Установить значение выходного сигнала тока
    state::state_output_current.dac_value_current = interpolated_value_of_viscosity / config::config_vvn8.getMultiplierViscosity() + 4.0 - config::config_vvn8.config_output_dac.current_alignment;
    set_dac_code();

    // Расчёт динамической вязкости
    // Значение динамической вязкости жидкости вычисляется делением  измеренной величины (Па*с*кг/м3)
    // на величину плотности контролируемой жидкости (кг/м3).
    // Вывести текущую динамическую вязкость во входное состояние
    state::state_input_current.viscosity_value_dynamic = interpolated_value_of_viscosity / state::state_input_current.density_value;


    // TODO: Вывести значение сырого кода АЦП для температуры в MODBUS-таблицу
    // TODO: Вывести отфильтрованного кода АЦП для температуры в MODBUS-таблицу
    // TODO: Вывести значение напряжения для вязкости в MODBUS-таблицу
    // TODO: вывести текущие кинематическую и динамическую вязкости в MODBUS-таблицу
}


void measure_temperature()
{
    ADMUX                = ADC_MUX_SELECT_TEMPERATURE;

    // Вывести значение сырого кода АЦП для температуры во входное состояние
    state::state_input_current.temperature_raw = adc::rawAnalogReadWithSleep();

    double filtered_value = static_cast<double>(calman::calmanContainer.temperature.add_and_filter_value(state::state_input_current.temperature_raw));

    // Вывести отфильтрованного кода АЦП для температуры во входное состояние
    int16_t filtered_temperature_adc_code = static_cast<int16_t>(round(filtered_value));
    CONSTRAINT_VAR_BY_ADC_LIMITS(filtered_temperature_adc_code);
    state::state_input_current.temperature_raw_filtered = static_cast<uint16_t>(filtered_temperature_adc_code);

    // 20ma: Vin = 2.42353, adc code = 969, calman filtered = 969.0
    // 4ma: Vin = 0.507235, adc code = 203, calman filtered = 203.0

    // Преобразовать в зачение тока 4-20 мА
    // интерполировать значение АЦП в температуру линейной интерполяцией
    double current_value;
    double _t;
    STATIC_LINEAR_INTERPOLATE(203, 4.0, 969, 20.0, _t, current_value, filtered_value);

    // выровнять значение тока
    current_value -= config::config_vvn8.config_temperature_sensor.current_alignment;

    // Вывести значение тока для температуры во входное состояние 
    state::state_input_current.temperature_current = current_value;

    // Вывести текущую температуру во входное состояние
    state::state_input_current.temperature_value = config::config_vvn8.config_temperature_sensor.interpolation_4_20ma.calculate(current_value);

    // TODO: Вывести значение сырого кода АЦП для температуры в MODBUS-таблицу
    // TODO: Вывести отфильтрованного кода АЦП для температуры в MODBUS-таблицу
    // TODO: Вывести значение тока для температуры в MODBUS-таблицу
    // TODO: вывести текущую температуру в MODBUS-таблицу
}


void measure_dencity()
{
    ADMUX                = ADC_MUX_SELECT_DENCITY;
    // Вывести значение сырого кода АЦП для плотности во входное состояние
    state::state_input_current.dencity_raw = adc::rawAnalogReadWithSleep();

    double filtered_value = static_cast<double>(calman::calmanContainer.dencity.add_and_filter_value(state::state_input_current.dencity_raw));

    // Вывести отфильтрованного кода АЦП для плотности во входное состояние
    int16_t filtered_dencity_adc_code = static_cast<int16_t>(round(filtered_value));
    CONSTRAINT_VAR_BY_ADC_LIMITS(filtered_dencity_adc_code);
    state::state_input_current.dencity_raw_filtered = static_cast<uint16_t>(filtered_dencity_adc_code);


    // 20ma: Vin = 2.42353, adc code = 969, calman filtered = 969.0
    // 4ma: Vin = 0.507235, adc code = 203, calman filtered = 203.0

    // Преобразовать в зачение тока 4-20 мА
    double current_value;
    double _t;
    STATIC_LINEAR_INTERPOLATE(203, 4.0, 969, 20.0, _t, current_value, filtered_value);


    // выровнять значение тока
    current_value -= config::config_vvn8.config_density_sensor.current_alignment;

    // Вывести значение тока для плотности во входное состояние 
    state::state_input_current.density_current = current_value;

    // Интерполировать значение тока АЦП в плотность  линейной интерполяцией
    state::state_input_current.density_value = config::config_vvn8.config_density_sensor.interpolation_4_20ma.calculate(current_value);

    // TODO: Вывести значение сырого кода АЦП для плотности в MODBUS-таблицу
    // TODO: Вывести отфильтрованного кода АЦП для плотности во входное состояние - и в MODBUS-таблицу
    // TODO: Вывести значение тока для плотности в MODBUS-таблицу
    // TODO: вывести текущую плотность в MODBUS-таблицу
}
