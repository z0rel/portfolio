/*
 * modbus.cpp
 *
 * Created: 10.03.2022 6:43:38
 *  Author: artem
 */ 


#include "../coroutine/Coroutine.h"
#include "../src/modbus/ModbusRtu_extern.h"


COROUTINE(task_modbus, void)
{
    COROUTINE_LOOP()
    {

        modbus_rtu::slave.poll(modbus_rtu::modbus_registers, modbus_rtu::modbus_coils);

        COROUTINE_DELAY_MAIN(1);
    }
}
