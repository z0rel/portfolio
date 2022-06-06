#ifndef MODBUS_RTU_H_
#define MODBUS_RTU_H_

/**
 * @file 	ModbusRtu.h
 * @version     1.21
 * @date        2016.02.21
 * @author 	Samuel Marco i Armengol
 * @contact     sammarcoarmengol@gmail.com
 * @contribution Helium6072
 * @contribution gabrielsan
 *
 * @description
 *  Arduino library for communicating with Modbus devices
 *  over RS232/USB/485 via RTU protocol.
 *
 *  Further information:
 *  http://modbus.org/
 *  http://modbus.org/docs/Modbus_over_serial_line_V1_02.pdf
 *
 */

#define MODBUS_DEBUG 1

#include <util/delay.h>

#include <inttypes.h>
#include "../../arduino/Arduino.h"
#include "../../arduino/likely.h"
#include "../vvn8_pins.h"
#include "ModbusRTU_messages.h"
#include "modbus_logic_proto.h"


namespace modbus_rtu {


/**
 * @struct modbus_t
 * @brief
 * Master query structure:
 * This includes all the necessary fields to make the Master generate a Modbus query.
 * A Master may keep several of these structures and send them cyclically or
 * use them according to program needs.
 */
typedef struct {
    /// Slave address between 1 and 247. 0 means broadcast
    uint8_t u8id;

    /// Function code: 1, 2, 3, 4, 5, 6, 15 or 16
    uint8_t u8fct;

    /// Address of the first register to access at slave/s
    uint16_t u16RegAdd;

    /// Number of coils or registers to access
    uint16_t u16CoilsNo;

    /// Pointer to memory image in master
    uint16_t *au16reg;
} modbus_t;


#define _MODBUS_TEMPLATE_LIST                                                                                \
    typename T_HARDWARE_SERIAL, uint16_t port_data_enable, uint8_t pin_data_enable, T_HARDWARE_SERIAL *port, \
      uint16_t modbus_buffer_length
#define _MODBUS_TEMPLATE_SPEC T_HARDWARE_SERIAL, port_data_enable, pin_data_enable, port, modbus_buffer_length


/**
 * @class Modbus
 * @brief
 * Arduino class library for communicating with Modbus devices over
 * USB/RS232/485 (via RTU protocol).
 */
template<_MODBUS_TEMPLATE_LIST>
class Modbus
{
  private:
    uint8_t u8id                            = 0;  //!< 0=master, 1..247=slave number
    uint8_t u8state                         = 0;
    uint8_t u8lastError                     = 0;
    uint8_t au8Buffer[modbus_buffer_length] = {0};
    uint8_t u8BufferSize                    = 0;
    uint8_t u8lastRec                       = 0;
    uint16_t u16InCnt                       = 0;
    uint16_t u16OutCnt                      = 0;
    uint16_t u16errCnt                      = 0;
    uint32_t u32time                        = 0;

    inline void sendTxBuffer();
    inline int8_t getRxBuffer();
    inline uint16_t calcCRC(uint8_t u8length);
    inline uint8_t validateAnswer();
    inline uint8_t validateRequest();
    inline int8_t process_FC1(uint16_t *regs);
    inline int8_t process_FC3(uint16_t *regs);
    inline int8_t process_FC5(uint16_t *regs);
    inline int8_t process_FC6(uint16_t *regs);
    inline int8_t process_FC15(uint16_t *regs);
    inline int8_t process_FC16(uint16_t *regs);

    /// build exception message
    inline void buildException(uint8_t u8exception);

  public:
    inline void init(uint8_t u8id) { this->u8id = u8id; }

    inline Modbus() {}
    inline Modbus(uint8_t u8id) { this->init(u8id); }


    inline void begin(long u32speed);

    // void begin(long u32speed, uint8_t u8config);
    inline void begin();

    /// only for master
    inline int8_t query(modbus_t telegram);

    /// cyclic poll for slave
    inline int8_t poll(uint16_t *regs, uint16_t *coils);

    /// number of incoming messages
    inline uint16_t getInCnt();

    /// number of outcoming messages
    inline uint16_t getOutCnt();

    /// error counter
    inline uint16_t getErrCnt();

    /// get slave ID between 1 and 247
    inline uint8_t getID();

    ///
    inline uint8_t getState();

    /// get last error message
    inline uint8_t getLastError();

    /// write new ID for the slave
    inline void setID(uint8_t u8id);

    /// finish any communication and release serial communication port
    // void end();
};


/**
 * @brief
 * Initialize class object.
 *
 * Sets up the serial port using specified baud rate.
 * Call once class has been instantiated, typically within setup().
 *
 * @see http://arduino.cc/en/Serial/Begin#.Uy4CJ6aKlHY
 * @param speed   baud rate, in standard increments (300..115200)
 * @ingroup setup
 */
template<_MODBUS_TEMPLATE_LIST>
inline void Modbus<_MODBUS_TEMPLATE_SPEC>::begin(long u32speed)
{
    port->begin(u32speed);
    // if (u8txenpin)  // pin 0 & pin 1 are reserved for RX/TX
    // {
    // return RS485 transceiver to transmit mode
    // pinMode(this->u8txenpin, OUTPUT);
    // digitalWrite(this->u8txenpin, LOW);
    // }

    while (port->not_empty()) {
        port->clear();
        _delay_us(1);
    }

    this->u8lastRec    = 0;
    this->u8BufferSize = 0;
    this->u16InCnt     = 0;
    this->u16OutCnt    = 0;
    this->u16errCnt    = 0;
}


/**
 * @brief
 * Initialize default class object.
 *
 * Sets up the serial port using 19200 baud.
 * Call once class has been instantiated, typically within setup().
 *
 * @overload Modbus::begin(uint16_t u16BaudRate)
 * @ingroup setup
 */
template<_MODBUS_TEMPLATE_LIST>
inline void Modbus<_MODBUS_TEMPLATE_SPEC>::begin()
{
    this->begin(19200);
}


/**
 * @brief
 * Method to write a new slave ID address
 *
 * @param 	u8id	new slave address between 1 and 247
 * @ingroup setup
 */
template<_MODBUS_TEMPLATE_LIST>
inline void Modbus<_MODBUS_TEMPLATE_SPEC>::setID(uint8_t u8id)
{
    if ((u8id != 0) && (u8id <= 247)) {
        this->u8id = u8id;
    }
}


/**
 * @brief
 * Method to read current slave ID address
 *
 * @return u8id	current slave address between 1 and 247
 * @ingroup setup
 */
template<_MODBUS_TEMPLATE_LIST>
inline uint8_t Modbus<_MODBUS_TEMPLATE_SPEC>::getID()
{
    return this->u8id;
}


/**
 * @brief
 * Get input messages counter value
 * This can be useful to diagnose communication
 *
 * @return input messages counter
 * @ingroup buffer
 */
template<_MODBUS_TEMPLATE_LIST>
inline uint16_t Modbus<_MODBUS_TEMPLATE_SPEC>::getInCnt()
{
    return this->u16InCnt;
}


/**
 * @brief
 * Get transmitted messages counter value
 * This can be useful to diagnose communication
 *
 * @return transmitted messages counter
 * @ingroup buffer
 */
template<_MODBUS_TEMPLATE_LIST>
inline uint16_t Modbus<_MODBUS_TEMPLATE_SPEC>::getOutCnt()
{
    return this->u16OutCnt;
}


/**
 * @brief
 * Get errors counter value
 * This can be useful to diagnose communication
 *
 * @return errors counter
 * @ingroup buffer
 */
template<_MODBUS_TEMPLATE_LIST>
inline uint16_t Modbus<_MODBUS_TEMPLATE_SPEC>::getErrCnt()
{
    return this->u16errCnt;
}


/**
 * Get modbus master state
 *
 * @return = 0 IDLE, = 1 WAITING FOR ANSWER
 * @ingroup buffer
 */
template<_MODBUS_TEMPLATE_LIST>
inline uint8_t Modbus<_MODBUS_TEMPLATE_SPEC>::getState()
{
    return this->u8state;
}


/**
 * Get the last error in the protocol processor
 *
 * @returnreturn   NO_REPLY = 255      Time-out
 * @return   EXC_FUNC_CODE = 1   Function code not available
 * @return   EXC_ADDR_RANGE = 2  Address beyond available space for Modbus registers
 * @return   EXC_REGS_QUANT = 3  Coils or registers number beyond the available space
 * @ingroup buffer
 */
template<_MODBUS_TEMPLATE_LIST>
inline uint8_t Modbus<_MODBUS_TEMPLATE_SPEC>::getLastError()
{
    return this->u8lastError;
}

/// TODO: redefine modbus rtu write function
/// size_t Print::write(const uint8_t *buffer, size_t size)
/// {
///     size_t n = 0;
///     while (size--) {
///         if (write(*buffer++))
///             n++;
///         else
///             break;
///     }
///     return n;
/// }


/**
 * @brief
 * This method transmits au8Buffer to Serial line.
 * Only if u8txenpin != 0, there is a flow handling in order to keep
 * the RS485 transceiver in output state as long as the message is being sent.
 * This is done with UCSRxA register.
 * The CRC is appended to the buffer before starting to send it.
 *
 * @param nothing
 * @return nothing
 * @ingroup buffer
 */
template<_MODBUS_TEMPLATE_LIST>
inline void Modbus<_MODBUS_TEMPLATE_SPEC>::sendTxBuffer()
{
    // append CRC to message
    uint16_t u16crc                     = this->calcCRC(this->u8BufferSize);
    this->au8Buffer[this->u8BufferSize] = u16crc >> 8;  // TODO: write as different digital write call
    this->u8BufferSize++;
    this->au8Buffer[this->u8BufferSize] = u16crc & 0x00ff;
    this->u8BufferSize++;

    // if (this->u8txenpin > 1) {
    // set RS485 transceiver to transmit mode

    (*reinterpret_cast<volatile uint8_t *>(port_data_enable)) |= _BV(pin_data_enable);  // digitalWrite(this->u8txenpin, HIGH);
    // }

    // transfer buffer to serial line
    // TODO: оптимизировать корутинной посимвольной записью


    uint8_t *buffer = this->au8Buffer;
    uint16_t n      = 0;
    uint16_t size   = this->u8BufferSize;
    while (size--) {
        if (port->write(*buffer++))
            n++;
        else
            break;
    }

    // port->write(this->au8Buffer, this->u8BufferSize);

    // Serial.println("Answer");
    // Serial.write(this->au8Buffer, this->u8BufferSize);


    // if (this->u8txenpin > 1) {
    // must wait transmission end before changing pin state
    // soft serial does not need it since it is blocking
    // if (this->u8serno < 4) {
    port->flush();
    // }
    // return RS485 transceiver to receive mode
    (*reinterpret_cast<volatile uint8_t *>(port_data_enable)) &= ~_BV(pin_data_enable);  // digitalWrite(this->u8txenpin, LOW);
    // }

    // TODO: make as coroutine
    // while (port->not_empty()) {
    //     port->clear();
    //     _delay_us(1);
    // }
    // while (port->read() >= 0) {  // TODO: сделать быстрый сброс головы вместо посимвольного вычитывания
    //     ;
    // }

    this->u8BufferSize = 0;

    // increase message counter
    this->u16OutCnt++;
}


/**
 * @brief
 * *** Only for Modbus Slave ***
 * This method checks if there is any incoming query
 * Afterwards, it would shoot a validation routine plus a register query
 * Avoid any delay() function !!!!
 * After a successful frame between the Master and the Slave, the time-out timer is reset.
 *
 * @param *regs  register table for communication exchange
 * @param u8size  size of the register table
 * @return 0 if no query, 1..4 if communication error, >4 if correct query processed
 * @ingroup loop
 */
template<_MODBUS_TEMPLATE_LIST>
inline int8_t Modbus<_MODBUS_TEMPLATE_SPEC>::poll(uint16_t *regs, uint16_t *coils)
{
    // check if there is any incoming frame
    uint8_t u8current = port->available();

    if (u8current == 0) {
        return 0;
    }
    // Serial.println("Query doing");

    // check T35 after frame end or still no frame end
    if (u8current != this->u8lastRec) {
        this->u8lastRec = u8current;
        this->u32time   = millis();
        return 0;
    }

#ifndef MODBUS_DEBUG
    if ((unsigned long)(millis() - this->u32time) < (unsigned long)T35) {
        return 0;
    }
#endif

    this->u8lastRec   = 0;
    int8_t i8state    = this->getRxBuffer();
    this->u8lastError = i8state;

    if (i8state < 7) {
        return i8state;
    }

    // check slave id
    if (this->au8Buffer[ID] != this->u8id) {
        return 0;
    }

    // validate message: CRC, FCT, address and size
    uint8_t u8exception = this->validateRequest();
    if (u8exception > 0) {
        if (u8exception != NO_REPLY) {
            this->buildException(u8exception);
            this->sendTxBuffer();
        }
        this->u8lastError = u8exception;
        return u8exception;
    }

    this->u8lastError = 0;

    // process message
    switch (static_cast<MB_FC>(this->au8Buffer[FUNC])) {
    case MB_FC::MB_FC_READ_COILS:
    case MB_FC::MB_FC_READ_DISCRETE_INPUT:
        return this->process_FC1(coils);
        break;
    case MB_FC::MB_FC_READ_INPUT_REGISTER:
    case MB_FC::MB_FC_READ_REGISTERS:
        return this->process_FC3(regs);
        break;
    case MB_FC::MB_FC_WRITE_COIL:
        return this->process_FC5(coils);
        break;
    case MB_FC::MB_FC_WRITE_REGISTER:
        return this->process_FC6(regs);
        break;
    case MB_FC::MB_FC_WRITE_MULTIPLE_COILS:
        return this->process_FC15(coils);
        break;
    case MB_FC::MB_FC_WRITE_MULTIPLE_REGISTERS:
        return this->process_FC16(regs);
        break;
    default:
        break;
    }
    return i8state;
}


/* _____PRIVATE FUNCTIONS_____________________________________________________ */


/**
 * @brief
 * This method moves Serial buffer data to the Modbus au8Buffer.
 *
 * @return buffer size if OK, ERR_BUFF_OVERFLOW if u8BufferSize >= modbus_buffer_length
 * @ingroup buffer
 */
template<_MODBUS_TEMPLATE_LIST>
inline int8_t Modbus<_MODBUS_TEMPLATE_SPEC>::getRxBuffer()
{
    // if (this->u8txenpin > 1) {
    (*reinterpret_cast<volatile uint8_t *>(port_data_enable)) &= ~_BV(pin_data_enable);  // digitalWrite(this->u8txenpin, LOW);
    // }

    this->u8BufferSize = 0;

    while (likely(port->available())) {
        this->au8Buffer[this->u8BufferSize] = port->read();
        ++this->u8BufferSize;

        if (unlikely(this->u8BufferSize >= modbus_buffer_length)) {
            this->u16InCnt++;
            this->u16errCnt++;
            return ERR_BUFF_OVERFLOW;
        }
    }
    this->u16InCnt++;

    return this->u8BufferSize;
}


/**
 * @brief
 * This method calculates CRC
 *
 * @return uint16_t calculated CRC value for the message
 * @ingroup buffer
 */
template<_MODBUS_TEMPLATE_LIST>
inline uint16_t Modbus<_MODBUS_TEMPLATE_SPEC>::calcCRC(uint8_t u8length)
{
    unsigned int temp, temp2, flag;
    temp = 0xFFFF;
    for (unsigned char i = 0; i < u8length; i++) {
        temp = temp ^ this->au8Buffer[i];
        for (unsigned char j = 1; j <= 8; j++) {
            flag = temp & 0x0001;
            temp >>= 1;
            if (flag) temp ^= 0xA001;
        }
    }
    // Reverse byte order.
    temp2 = temp >> 8;
    temp  = (temp << 8) | temp2;
    temp &= 0xFFFF;
    // the returned value is already swapped
    // crcLo byte is first & crcHi byte is last
    return temp;
}


/**
 * @brief
 * This method validates slave incoming messages
 *
 * @return 0 if OK, EXCEPTION if anything fails
 * @ingroup buffer
 */
template<_MODBUS_TEMPLATE_LIST>
inline uint8_t Modbus<_MODBUS_TEMPLATE_SPEC>::validateRequest()
{
    // Проверить сообщение контрольной суммы vs рассчитанную CRC
    // скомбинировать нижний и верхний байты CRC
    uint16_t u16MsgCRC = ((this->au8Buffer[this->u8BufferSize - 2] << 8) | this->au8Buffer[this->u8BufferSize - 1]);

    if (this->calcCRC(this->u8BufferSize - 2) != u16MsgCRC) {
        this->u16errCnt++;
        return NO_REPLY;
    }

    // Проверить код fct
    switch (static_cast<MB_SUPPORTED>(this->au8Buffer[FUNC])) {
    case MB_SUPPORTED::MB_FC_READ_COILS:
    case MB_SUPPORTED::MB_FC_READ_DISCRETE_INPUT:
    case MB_SUPPORTED::MB_FC_READ_REGISTERS:
    case MB_SUPPORTED::MB_FC_READ_INPUT_REGISTER:
    case MB_SUPPORTED::MB_FC_WRITE_COIL:
    case MB_SUPPORTED::MB_FC_WRITE_REGISTER:
    case MB_SUPPORTED::MB_FC_WRITE_MULTIPLE_COILS:
    case MB_SUPPORTED::MB_FC_WRITE_MULTIPLE_REGISTERS:
        break;
    default:
        this->u16errCnt++;
        return EXC_FUNC_CODE;
    }

    // Проверить стартовый адрес и & nb диапазон
    uint16_t u16regs = 0;
    switch (static_cast<MB_FC>(this->au8Buffer[FUNC])) {
    case MB_FC::MB_FC_NONE:
        break;
    case MB_FC::MB_FC_READ_COILS:
    case MB_FC::MB_FC_READ_DISCRETE_INPUT:
        u16regs = word(this->au8Buffer[ADD_HI], this->au8Buffer[ADD_LO]);
        u16regs += word(this->au8Buffer[NB_HI], this->au8Buffer[NB_LO]);
        if (u16regs > RegistersLogicCoils::_COIL_MAX) {
            return EXC_ADDR_RANGE;
        }
        break;
    case MB_FC::MB_FC_WRITE_MULTIPLE_COILS:
        u16regs = word(this->au8Buffer[ADD_HI], this->au8Buffer[ADD_LO]);
        u16regs += word(this->au8Buffer[NB_HI], this->au8Buffer[NB_LO]);
        if (u16regs > RegistersLogicCoils::_COIL_MAX) {
            return EXC_ADDR_RANGE;
        }
        break;
    case MB_FC::MB_FC_WRITE_COIL:
        u16regs = word(this->au8Buffer[ADD_HI], this->au8Buffer[ADD_LO]);
        if (u16regs > RegistersLogicCoils::_COIL_MAX) {
            return EXC_ADDR_RANGE;
        }
        break;
    case MB_FC::MB_FC_WRITE_REGISTER:
        u16regs = word(this->au8Buffer[ADD_HI], this->au8Buffer[ADD_LO]);
        if (u16regs > RegistersLogic::_TABLE_SIZE) {
            return EXC_ADDR_RANGE;
        }
        break;
    case MB_FC::MB_FC_READ_REGISTERS:
    case MB_FC::MB_FC_READ_INPUT_REGISTER:
        u16regs = word(this->au8Buffer[ADD_HI], this->au8Buffer[ADD_LO]);
        u16regs += word(this->au8Buffer[NB_HI], this->au8Buffer[NB_LO]);
        if (u16regs > RegistersLogic::_TABLE_SIZE) {
            return EXC_ADDR_RANGE;
        }
        break;
    case MB_FC::MB_FC_WRITE_MULTIPLE_REGISTERS:
        u16regs = word(this->au8Buffer[ADD_HI], this->au8Buffer[ADD_LO]);
        u16regs += word(this->au8Buffer[NB_HI], this->au8Buffer[NB_LO]);
        if (u16regs > RegistersLogic::_TABLE_SIZE) {
            return EXC_ADDR_RANGE;
        }
        break;
    }
    return 0;  // OK, no exception code thrown
}


/**
 * @brief
 * This method validates master incoming messages
 *
 * @return 0 if OK, EXCEPTION if anything fails
 * @ingroup buffer
 */
template<_MODBUS_TEMPLATE_LIST>
inline uint8_t Modbus<_MODBUS_TEMPLATE_SPEC>::validateAnswer()
{
    // check message CRC vs calculated CRC
    // combine the crc Low & High bytes
    uint16_t u16MsgCRC = ((this->au8Buffer[this->u8BufferSize - 2] << 8) | this->au8Buffer[this->u8BufferSize - 1]);

    if (this->calcCRC(this->u8BufferSize - 2) != u16MsgCRC) {
        this->u16errCnt++;
        return NO_REPLY;
    }

    // Проверить исключение
    if ((this->au8Buffer[FUNC] & 0x80) != 0) {
        this->u16errCnt++;
        return ERR_EXCEPTION;
    }

    // Проверить код fct
    switch (static_cast<MB_SUPPORTED>(this->au8Buffer[FUNC])) {
    case MB_SUPPORTED::MB_FC_READ_COILS:
    case MB_SUPPORTED::MB_FC_READ_DISCRETE_INPUT:
    case MB_SUPPORTED::MB_FC_READ_REGISTERS:
    case MB_SUPPORTED::MB_FC_READ_INPUT_REGISTER:
    case MB_SUPPORTED::MB_FC_WRITE_COIL:
    case MB_SUPPORTED::MB_FC_WRITE_REGISTER:
    case MB_SUPPORTED::MB_FC_WRITE_MULTIPLE_COILS:
    case MB_SUPPORTED::MB_FC_WRITE_MULTIPLE_REGISTERS:
        return 0;  // OK, no exception code thrown
    default:
        this->u16errCnt++;
        return EXC_FUNC_CODE;
    }

    return 0;  // OK, no exception code thrown
}

/**
 * @brief
 * This method builds an exception message
 *
 * @ingroup buffer
 */
template<_MODBUS_TEMPLATE_LIST>
inline void Modbus<_MODBUS_TEMPLATE_SPEC>::buildException(uint8_t u8exception)
{
    uint8_t u8func = this->au8Buffer[FUNC];  // get the original FUNC code

    this->au8Buffer[ID]   = this->u8id;
    this->au8Buffer[FUNC] = u8func + 0x80;
    this->au8Buffer[2]    = u8exception;
    this->u8BufferSize    = EXCEPTION_SIZE;
}


/**
 * @brief
 * This method processes functions 1 & 2
 * This method reads a bit array and transfers it to the master
 *
 * @return this->u8BufferSize Response to master length
 * @ingroup discrete
 */
template<_MODBUS_TEMPLATE_LIST>
inline int8_t Modbus<_MODBUS_TEMPLATE_SPEC>::process_FC1(uint16_t *regs)
{
    // получить первую и последнюю coil из сообщения
    uint16_t u16StartCoil = word(this->au8Buffer[ADD_HI], this->au8Buffer[ADD_LO]);
    uint16_t u16Coilno    = word(this->au8Buffer[NB_HI], this->au8Buffer[NB_LO]);

    // установить количество байтов в исходящем сообщении
    uint8_t u8bytesno = static_cast<uint8_t>(u16Coilno >> 3);  // u16Coilno / 8
    if ((u16Coilno & 7) != 0) {  // u16Coilno % 8 != 0
        u8bytesno++;
    }
    this->au8Buffer[ADD_HI]                             = u8bytesno;
    this->u8BufferSize                                  = ADD_LO;
    this->au8Buffer[this->u8BufferSize + u8bytesno - 1] = 0;

    // прочитать каждую coil из карты регистров и поместить ее значение в исходящее сообщение
    uint8_t u8bitsno = 0;

    for (uint16_t i = 0; i < u16Coilno; ++i) {
        uint16_t u16coil          = u16StartCoil + i;
        uint8_t u8currentRegister = static_cast<uint8_t>(u16coil >> 4);  // u16coil / 16
        uint8_t u8currentBit      = static_cast<uint8_t>(u16coil & 15);  // u16coil % 16
        bitWrite(this->au8Buffer[this->u8BufferSize], u8bitsno, bitRead(regs[u8currentRegister], u8currentBit));
        u8bitsno++;

        if (u8bitsno > 7) {
            u8bitsno = 0;
            this->u8BufferSize++;
        }
    }

    // send outcoming message
    if ((u16Coilno & 7) != 0) {  // u16Coilno % 8 != 0
        this->u8BufferSize++;
    }
    uint8_t u8CopyBufferSize = this->u8BufferSize + 2;
    this->sendTxBuffer();
    return u8CopyBufferSize;
}


/**
 * @brief
 * This method processes functions 3 & 4
 * This method reads a word array and transfers it to the master
 *
 * @return this->u8BufferSize Response to master length
 * @ingroup register
 */
template<_MODBUS_TEMPLATE_LIST>
inline int8_t Modbus<_MODBUS_TEMPLATE_SPEC>::process_FC3(uint16_t *regs)
{
    uint8_t u8StartAddr = word(this->au8Buffer[ADD_HI], this->au8Buffer[ADD_LO]);
    uint8_t u8regsno    = word(this->au8Buffer[NB_HI], this->au8Buffer[NB_LO]);

    // TODO: вместо заполнения this->au8Buffer - сделать прямой расчет CRC и побитовый вывод
    this->au8Buffer[2] = u8regsno << 1;  // * 2
    this->u8BufferSize = 3;

    for (uint8_t i = u8StartAddr; i < u8StartAddr + u8regsno; ++i) {
        this->au8Buffer[this->u8BufferSize] = highByte(regs[i]);
        this->u8BufferSize++;
        this->au8Buffer[this->u8BufferSize] = lowByte(regs[i]);
        this->u8BufferSize++;
    }
    uint8_t u8CopyBufferSize = this->u8BufferSize + 2;

    uint16_t oldsize = this->u8BufferSize + 2;

    this->sendTxBuffer();

	Serial.print(static_cast<int>(u8StartAddr));
	Serial.print(' ');
	Serial.print(static_cast<int>(u8regsno));
	Serial.print(' ');
	Serial.print(" len: ");
	Serial.print(oldsize);
	Serial.print(" msg: ");
	for (uint16_t ii = 0; ii < oldsize; ++ii) {
	    Serial.print(static_cast<int>(this->au8Buffer[ii]));
	    Serial.print(' ');
	}
	Serial.println(' ');
	Serial.println(' ');

    return u8CopyBufferSize;
}


/**
 * @brief
 * This method processes function 5
 * This method writes a value assigned by the master to a single bit
 *
 * @return this->u8BufferSize Response to master length
 * @ingroup discrete
 */
template<_MODBUS_TEMPLATE_LIST>
inline int8_t Modbus<_MODBUS_TEMPLATE_SPEC>::process_FC5(uint16_t *regs)
{
    uint8_t u8currentRegister, u8currentBit;
    uint8_t u8CopyBufferSize;
    uint16_t u16coil = word(this->au8Buffer[ADD_HI], this->au8Buffer[ADD_LO]);

    // point to the register and its bit
    u8currentRegister = static_cast<uint8_t>(u16coil >> 4);  // u16coil / 16
    u8currentBit      = static_cast<uint8_t>(u16coil & 15);  // u16coil % 16

    // write to coil
    bitWrite(regs[u8currentRegister], u8currentBit, this->au8Buffer[NB_HI] == 0xff);

    // send answer to master
    this->u8BufferSize = 6;
    u8CopyBufferSize   = this->u8BufferSize + 2;
    this->sendTxBuffer();

    return u8CopyBufferSize;
}


/**
 * @brief
 * This method processes function 6
 * This method writes a value assigned by the master to a single word
 *
 * @return this->u8BufferSize Response to master length
 * @ingroup register
 */
template<_MODBUS_TEMPLATE_LIST>
inline int8_t Modbus<_MODBUS_TEMPLATE_SPEC>::process_FC6(uint16_t *regs)
{
    uint8_t u8add = word(this->au8Buffer[ADD_HI], this->au8Buffer[ADD_LO]);
    uint8_t u8CopyBufferSize;
    uint16_t u16val = word(this->au8Buffer[NB_HI], this->au8Buffer[NB_LO]);

    regs[u8add] = u16val;

    // keep the same header
    this->u8BufferSize = RESPONSE_SIZE;

    u8CopyBufferSize = this->u8BufferSize + 2;
    this->sendTxBuffer();

    return u8CopyBufferSize;
}


/**
 * @brief
 * This method processes function 15
 * This method writes a bit array assigned by the master
 *
 * @return this->u8BufferSize Response to master length
 * @ingroup discrete
 */
template<_MODBUS_TEMPLATE_LIST>
inline int8_t Modbus<_MODBUS_TEMPLATE_SPEC>::process_FC15(uint16_t *regs)
{
    // get the first and last coil from the message
    uint16_t u16StartCoil = word(this->au8Buffer[ADD_HI], this->au8Buffer[ADD_LO]);
    uint16_t u16Coilno    = word(this->au8Buffer[NB_HI], this->au8Buffer[NB_LO]);


    // read each coil from the register map and put its value inside the outcoming message
    uint8_t u8bitsno    = 0;
    uint8_t u8frameByte = 7;
    for (uint16_t i = 0; i < u16Coilno; ++i) {
        uint16_t u16coil          = u16StartCoil + i;
        uint8_t u8currentRegister = static_cast<uint8_t>(u16coil >> 4);  // u16coil / 16
        uint8_t u8currentBit      = static_cast<uint8_t>(u16coil & 15);  // u16coil % 16

        boolean bTemp = bitRead(this->au8Buffer[u8frameByte], u8bitsno);

        bitWrite(regs[u8currentRegister], u8currentBit, bTemp);

        u8bitsno++;

        if (u8bitsno > 7) {
            u8bitsno = 0;
            u8frameByte++;
        }
    }

    // send outcoming message
    // it's just a copy of the incomping frame until 6th byte
    this->u8BufferSize       = 6;
    uint8_t u8CopyBufferSize = this->u8BufferSize + 2;

    this->sendTxBuffer();

    return u8CopyBufferSize;
}


/**
 * @brief
 * This method processes function 16
 * This method writes a word array assigned by the master
 *
 * @return this->u8BufferSize Response to master length
 * @ingroup register
 */
template<_MODBUS_TEMPLATE_LIST>
inline int8_t Modbus<_MODBUS_TEMPLATE_SPEC>::process_FC16(uint16_t *regs)
{
    uint16_t u8StartAdd = this->au8Buffer[ADD_HI] << 8 | this->au8Buffer[ADD_LO];
    uint16_t u8regsno   = this->au8Buffer[NB_HI] << 8 | this->au8Buffer[NB_LO];

    // build header
    this->au8Buffer[NB_HI] = 0;
    this->au8Buffer[NB_LO] = u8regsno;
    this->u8BufferSize     = RESPONSE_SIZE;

    // write registers
    for (uint8_t i = 0; i < u8regsno; i++) {
        uint16_t temp = word(this->au8Buffer[(BYTE_CNT + 1) + (i << 1)], this->au8Buffer[(BYTE_CNT + 2) + (i << 1)]);

        regs[u8StartAdd + i] = temp;
    }
    uint8_t u8CopyBufferSize = this->u8BufferSize + 2;

    this->sendTxBuffer();

    return u8CopyBufferSize;
}


}  // namespace modbus_rtu


#endif  // MODBUS_RTU_H_
