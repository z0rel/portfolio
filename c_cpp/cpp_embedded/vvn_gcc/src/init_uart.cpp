/*
 * uart_init.cpp
 *
 * Created: 01.03.2022 8:57:14
 *  Author: artem
 */


#include "init_uart.h"

#include "../arduino/pins_arduino.h"
#include "../arduino/Arduino.h"
#include "../arduino/wiring_private.h"
#include "types_dictionary.h"
#include "modbus/ModbusRtu_extern.h"


#define BAUD_CALC(x) ((F_CPU + (x)*8UL) / (16UL * (x)) - 1UL)  // macro calculating precise UBRR value
#define BAUD_CALC_FAST(x) ((F_CPU) / ((x)*16UL) - 1)  // for faster real time calculations ? // not recommended
#define DOUBLE_BAUD_CALC(x) ((F_CPU + (x)*4UL) / (8UL * (x)) - 1UL)  // macro calculating UBRR value for double speed


void init_uart()
{
    uint16_t ubrr_value  = BAUD_CALC(115200);
    uint8_t uscr1c_flags = 0;

    config::ModbusConfig modbus_config;
    modbus_config.load();

    // Настройка чётности
    switch (modbus_config.parity) {
    case config::UART_Parity::NONE:
        // UCSR1C &= ~(_BV(UPM11) | _BV(UPM10));  // Без контроля чётности
        break;
    case config::UART_Parity::ODD:  // нечётное
        uscr1c_flags = _BV(UPM11) | _BV(UPM10);  // Контроль нечётности
        break;
    case config::UART_Parity::EVEN:  // чётное
        uscr1c_flags = _BV(UPM11);
        // UCSR1C &= ~_BV(UPM10);  // Контроль чётности
        break;
    default:
        break;
    }

    // Настройка стоп-бит
    switch (modbus_config.stopbits) {
    case config::UART_Stopbits::ONE:
        // UCSR1C &= ~_BV(USBS1);  // 1 стоп-бит
        break;
    case config::UART_Stopbits::TWO:
        uscr1c_flags |= _BV(USBS1);  // 2 стоп-бита
        break;
    default:
        break;
    }

    // Настройка длины данных
    switch (modbus_config.data_bits) {
    case config::UART_DataBits::B_5:
        // UCSR1C &= ~(_BV(UCSZ10) | _BV(UCSZ11) | _BV(UCSZ12));
        break;
    case config::UART_DataBits::B_6:  // 0 0 1
        uscr1c_flags |= _BV(UCSZ10);
        // UCSR1C &= ~(_BV(UCSZ11) | _BV(UCSZ12));
        break;
    case config::UART_DataBits::B_7:  // 0 1 0
        uscr1c_flags |= _BV(UCSZ11);
        // UCSR1C &= ~(_BV(UCSZ10) | _BV(UCSZ12));
        break;
    case config::UART_DataBits::B_8:  // 0 1 1
        uscr1c_flags |= (_BV(UCSZ10) | _BV(UCSZ11));
        // UCSR1C &= ~_BV(UCSZ12);
        break;
    case config::UART_DataBits::B_9:
        uscr1c_flags |= (_BV(UCSZ10) | _BV(UCSZ11) | _BV(UCSZ12));
        break;
    default:
        break;
    }

    switch (modbus_config.baudrate) {
    case config::UART_Baudrate::B_2400:
        ubrr_value = BAUD_CALC(2400);
        break;
    case config::UART_Baudrate::B_4800:
        ubrr_value = BAUD_CALC(4800);
        break;
    case config::UART_Baudrate::B_9600:
        ubrr_value = BAUD_CALC(9600);
        break;
    case config::UART_Baudrate::B_14400:
        ubrr_value = BAUD_CALC(14400);
        break;
    case config::UART_Baudrate::B_19200:
        ubrr_value = BAUD_CALC(19200);
        break;
    case config::UART_Baudrate::B_28800:
        ubrr_value = BAUD_CALC(28800);
        break;
    case config::UART_Baudrate::B_38400:
        ubrr_value = BAUD_CALC(38400);
        break;
    case config::UART_Baudrate::B_57600:
        ubrr_value = BAUD_CALC(57600);
        break;
    case config::UART_Baudrate::B_76800:
        ubrr_value = BAUD_CALC(76800);
        break;
    case config::UART_Baudrate::B_115200:
        ubrr_value = BAUD_CALC(115200);
        break;
    default:
        break;
    }

    UCSR1B = 0;  // flush all hardware buffers
    uscr1c_flags = (_BV(UCSZ10) | _BV(UCSZ11)); // for debug
    UCSR1C = uscr1c_flags;
    // (writing TXENn to zero) will not become effective until ongoing and pending transmissions are completed

    
    ubrr_value = BAUD_CALC(115200); // for debug
    UBRR1L = (uint8_t)ubrr_value;
    UBRR1H = (ubrr_value >> 8);

    sbi(UCSR1B, RXEN0);
    sbi(UCSR1B, TXEN0);
    sbi(UCSR1B, RXCIE0);
    cbi(UCSR1B, UDRIE0);

    modbus_config.device_number = 1; // for debug;
    modbus_rtu::slave.init(modbus_config.device_number);


    UCSR0B = 0;  // flush all hardware buffers
    // UCSR0C &= ~(_BV(UPM11) | _BV(UPM10));  // Без контроля чётности
    // UCSR0C &= ~_BV(USBS1);  // 1 стоп-бит
    UCSR0C = (_BV(UCSZ10) | _BV(UCSZ11));  // 0 1 1 - 8 бит данных
    // UCSR0C &= ~_BV(UCSZ12); // 0 1 1 - 8 бит данных
    ubrr_value = BAUD_CALC(115200);
    UBRR0L     = (uint8_t)ubrr_value;
    UBRR0H     = (ubrr_value >> 8);

    sbi(UCSR0B, RXEN0);
    sbi(UCSR0B, TXEN0);
    sbi(UCSR0B, RXCIE0);
    cbi(UCSR0B, UDRIE0);

    // Отладочный COM-порт
    // Serial.begin(9600  //,
    //              /* Без контроля чётности */
    //              // (_BV(UCSZ10) | _BV(UCSZ11)) /* 8 бит данных */
    //              /* 1 стоп-бит */
    // );
}
