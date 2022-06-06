/*
 * config_modbus.h
 *
 * Created: 09.04.2022 5:08:13
 *  Author: artem
 */


#ifndef CONFIG_MODBUS_H_
#define CONFIG_MODBUS_H_


namespace config {


enum class UART_Baudrate : uint8_t {
    B_2400   = 0,
    B_4800   = 1,
    B_9600   = 2,
    B_14400  = 3,
    B_19200  = 4,
    B_28800  = 5,
    B_38400  = 6,
    B_57600  = 7,
    B_76800  = 8,
    B_115200 = 9
};


enum class UART_DataBits : uint8_t { B_5 = 0, B_6 = 1, B_7 = 2, B_8 = 3, B_9 = 4 };


enum class UART_Parity : uint8_t { NONE = 0, ODD = 1, EVEN = 2 };


enum class UART_Stopbits : uint8_t { ONE = 0, TWO = 1 };


/// Настройки Modbus
class ModbusConfig
{
  public:
    /// Бодрейт
    UART_Baudrate baudrate = UART_Baudrate::B_115200;
    /// Число бит данных
    UART_DataBits data_bits = UART_DataBits::B_8;  // const
    /// Контроль чётности
    UART_Parity parity = UART_Parity::NONE;  // const
                                                  /// Число стоп бит
    UART_Stopbits stopbits = UART_Stopbits::ONE;  // const
    /// Номер устройства Modbus
    uint8_t device_number = 1;

    void load();
    void save();
};


}  // namespace config


#endif /* CONFIG_MODBUS_H_ */
