/*
 * init_adc.cpp
 *
 * Created: 02.03.2022 0:01:09
 *  Author: artem
 */


#include <avr/io.h>
#include <avr/sfr_defs.h>

#include "../arduino/Arduino.h"
#include "init_adc.h"

#include "../arduino/pins_arduino.h"


// Включить пины АЦП ADC5, ADC6, ADC7:
// MUX4..0   ADC
// MUX4 = 0; MUX3 = 0; MUX2 = 0; MUX1 = 0; MUX0 = 0     ADC0
// MUX4 = 0; MUX3 = 0; MUX2 = 0; MUX1 = 0; MUX0 = 1     ADC1
// MUX4 = 0; MUX3 = 0; MUX2 = 0; MUX1 = 1; MUX0 = 0     ADC2
// MUX4 = 0; MUX3 = 0; MUX2 = 0; MUX1 = 1; MUX0 = 1     ADC3
// MUX4 = 0; MUX3 = 0; MUX2 = 1; MUX1 = 0; MUX0 = 0     ADC4
// MUX4 = 0; MUX3 = 0; MUX2 = 1; MUX1 = 0; MUX0 = 1     ADC5 *
// MUX4 = 0; MUX3 = 0; MUX2 = 1; MUX1 = 1; MUX0 = 0     ADC6 *
// MUX4 = 0; MUX3 = 0; MUX2 = 1; MUX1 = 1; MUX0 = 1     ADC7 *




// Настройка АЦП
void init_adc()
{
    ADCSRA = 0; // сбрасываем АЦП

    // DISABLE DIGITAL INPUTS
    // только для atmega328
    // DIDR0  |= _BV(ADC5D) | _BV(ADC4D) | _BV(ADC3D) |  _BV(ADC2D) | _BV(ADC1D) | _BV(ADC0D);

    // - Выбор опорного напряжения АЦП как AVCC: REFS1 = 0, REFS0 = 1
    // Выбор опорного напряжения АЦП как INTERNAL REFERENCE 2.56V: REFS1 = 1, REFS0 = 1
    // Выравнивание результата по правому краю (ADLAR = 0)
	// Выбор начального входного канала измерения вязкости
    ADMUX = ADC_MUX_SELECT_VISCOSITY;

    // adc_enable_p_vvn_adc7();

    // Default prescaler is 57.6

    // Разрешение АЦП: До 76,9 тыс выборок в секунду (до 15 тыс выборок в секунду при максимальном разрешении)

    // F_CPU = 7372800UL = 7.3728Mhz
    // Prescaler = 128, ADC Frequency = 57.6 kHz (7372.8 kHz / 128)
    // Prescaler = 64, ADC Frequency = 115.2 kHz (7372.8 kHz / 64)

    // ADPS2     ADPS1     ADPS0     Division Factor
    // ADPS2 = 0 ADPS1 = 0 ADPS0 = 0 Division Factor = 2
    // ADPS2 = 0 ADPS1 = 0 ADPS0 = 1 Division Factor = 2
    // ADPS2 = 0 ADPS1 = 1 ADPS0 = 0 Division Factor = 4
    // ADPS2 = 0 ADPS1 = 1 ADPS0 = 1 Division Factor = 8
    // ADPS2 = 1 ADPS1 = 0 ADPS0 = 0 Division Factor = 16
    // ADPS2 = 1 ADPS1 = 0 ADPS0 = 1 Division Factor = 32
    // ADPS2 = 1 ADPS1 = 1 ADPS0 = 0 Division Factor = 64
    // ADPS2 = 1 ADPS1 = 1 ADPS0 = 1 Division Factor = 128
	// В за одну выборку АЦП делает 13 отсчетов
	// (7372800/128)/13 = 4430
	// Если АЦП будет работать непрерывно, в секунду можно получить 4430 отсчета, т.е. одно измерение занимает 225 мкс 
	// в худшем случае, при деградации производительности берем число не 13, а 62.5 
	// В этом случае - АЦП будет делать ((7372800/128)/62.5)=922 отсчетов в секунду, т.е. одно измерение будет занимать 1,085 мс
	
    ADCSRA |= (_BV(ADEN) // Включить ADC
               | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0) // Установить предделитель в 128 (ADPS2 = 1 ADPS1 = 1 ADPS0 = 1), ADC Frequency = 57.6 kHz
               | _BV(ADIE) // Включить прерывания АЦП
    );

    // Отключить автоконверсию АЦП (ADFR: ADC Free Running Select)
    ADCSRA &= ~_BV(ADFR);

    // TODO: не нужно, если чтение результатов преобразования будет осуществляться не через analogRead библиотеки arduino
    // analogReference(DEFAULT);
}
