/*
 * adc_coroutine.cpp
 *
 * Created: 29.03.2022 7:39:38
 *  Author: artem
 */


#include "../../arduino/Arduino.h"
#include "../../coroutine/Coroutine.h"

#include "adc_measurement.h"


COROUTINE(task_adc, void)
{
    COROUTINE_LOOP()
    {
        measure_dencity(); // Сначала измеряется плотность, она нужна для расчета динамической вязкости
        measure_viscosity();
        measure_temperature();

        COROUTINE_DELAY_MAIN(100);
    }
}
