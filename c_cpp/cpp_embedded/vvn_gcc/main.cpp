/*
 * vvn_gcc.cpp
 *
 * Created: 24.02.2022 1:10:22
 * Author : artem
 */

#include <avr/io.h>
#include <util/delay.h>

#include "arduino/Arduino.h"
#include "arduino/pins_arduino.h"

#include "freertos/heap.h"

#include "coroutine/task.h"

#include "coroutines/coroutines.h"
#include "coroutines/display_context.h"

#include "src/dac.h"
#include "src/init_adc.h"
#include "src/init_extram.h"
#include "src/init_gpio.h"
#include "src/init_uart.h"
#include "src/rtc.h"
#include "src/types_dictionary.h"
#include "src/vvn8_pins.h"

#include "src/config_vvn8/config_vvn8.h"
#include "src/lcd/lcd.h"
#include "src/state/state.h"

#include "src/modbus/ModbusRtu.h"


void first_init_eeprom_if_need()
{
    if (eeprom_read_byte(&eeprom_first_initialization_key) != EEPROM_FIRST_INITIALIZATION_KEY) { config::first_init_eeprom(); }
}


void setup()
{
    init();  // Arduino Initialization of timers
    init_adc();
    first_init_eeprom_if_need();
    init_gpio();
    init_uart();
    init_extram();
    init_dac();
    rtc_init();

    state::set_initial_state();
}


multitasking::Task<8> ctx_task_display(&task_display, &coroutines::displayContext);
multitasking::Task<2> ctx_state_next(&state::state_next);
multitasking::Task<2> ctx_task_adc(&task_adc);
multitasking::Task<4> ctx_task_modbus(&task_modbus);


int main(void)
{
    setup();

    while (1) {
        ctx_task_display.run();
        ctx_state_next.run();
        // АЦП оцифровка с трёх входов и фильтрация Калманом
        ctx_task_adc.run();
        ctx_task_modbus.run();
    }
}
