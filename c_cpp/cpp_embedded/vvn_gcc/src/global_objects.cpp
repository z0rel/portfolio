
#include <avr/eeprom.h>

#include "../coroutine/GlobalLockAdc_int.h"
#include "config_vvn8/config_vvn8.h"
#include "rtc.h"
#include "types_dictionary_int.h"

#include "../coroutine/task.h"
#include "../coroutines/adc/adc_utils.h"
#include "../coroutines/display_context.h"
#include "../filter/fileter_container.h"

#include "modbus/modbus_logic_proto.h"
#include "modbus/ModbusRtu.h"
#include "state/hmi_state.h"

EepromFirstInitializationKey EEMEM eeprom_first_initialization_key;


namespace config {
  ConfigVVN8 config_vvn8;
}


#define MAX_ADC_SOURCES 4

namespace multitasking {
VStackArray *current_task = nullptr;
}

/// Результаты измерений с разных источников данных АЦП
volatile AdcResult adc_measured_voltage_interrupt_results[MAX_ADC_SOURCES] = {0, 0, 0, 0};


/// Тип глобальной блокировки
typedef uint8_t GlobalLock;

/// Флаг глобальной блокировки
volatile GlobalLock global_lock = 0;


/// Константа для выбора Internal Vref при оцифровке
#define ADC_INTERNAL_VOLTAGE_REF_FLAG _BV(REFS0) | _BV(REFS1)


namespace state {

HmiStateInput state_input_current;

HmiStateOutput state_output_current;
HmiStateOutput state_output_previous;


} // namespace state

namespace coroutines {
DisplayContext displayContext;
}

namespace adc {
ADC_measurement ADC_measurement_status;
}

namespace calman {
CalmanContainer calmanContainer;
}

namespace modbus_rtu {
uint16_t modbus_registers[static_cast<uint8_t>(modbus_rtu::RegistersLogic::_TABLE_SIZE)] = {0};
// Modbus<&(P_DEN_PORT), (P_DEN_PIN), (modbus_rtu::RegistersLogic::_TABLE_SIZE), &Serial1> slave;
uint16_t modbus_coils[static_cast<uint8_t>(RegistersLogicCoils::_COIL_MAX_TABLE_SIZE)];
Modbus<HardwareSerial1, reinterpret_cast<uint16_t>(&(P_DEN_PORT)), P_DEN_PIN, &Serial1, modbus_rtu::RegistersLogic::_MAX_MESSAGE_SIZE> slave;

}

// RealTimeClockInternal __attribute__((section(".rtcsec"))) rtc_struct;
// uint8_t __attribute__((section(".calmanbuf"))) calman_buffer[4352];
