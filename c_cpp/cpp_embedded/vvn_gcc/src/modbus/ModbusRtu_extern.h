/*
 * ModbusRtu_extern.h
 *
 * Created: 14.04.2022 18:15:40
 *  Author: artem
 */


#ifndef MODBUSRTU_EXTERN_H_
#define MODBUSRTU_EXTERN_H_

#include "ModbusRtu.h"
#include "../../arduino/HardwareSerial.h"
#include "modbus_logic_proto.h"


namespace modbus_rtu {


extern Modbus<HardwareSerial1,
              reinterpret_cast<uint16_t>(&(P_DEN_PORT)),
              P_DEN_PIN,
              &Serial1,
              modbus_rtu::RegistersLogic::_MAX_MESSAGE_SIZE>
  slave;
extern uint16_t modbus_registers[static_cast<uint8_t>(RegistersLogic::_TABLE_SIZE)];
extern uint16_t modbus_coils[static_cast<uint8_t>(RegistersLogicCoils::_COIL_MAX_TABLE_SIZE)];

}  // namespace modbus_rtu


#endif /* MODBUSRTU_EXTERN_H_ */
