/*
 * ModbusRTU_master.h
 *
 * Created: 09.04.2022 15:22:35
 *  Author: artem
 */


#ifndef MODBUSRTU_MASTER_H_
#define MODBUSRTU_MASTER_H_


namespace modbus_rtu {


/**
 * @brief
 * *** Only Modbus Master ***
 * Generate a query to an slave with a modbus_t telegram structure
 * The Master must be in COM_IDLE mode. After it, its state would be COM_WAITING.
 * This method has to be called only in loop() section.
 *
 * @see modbus_t
 * @param modbus_t  modbus telegram structure (id, fct, ...)
 * @ingroup loop
 * @todo finish function 15
 */
template<_MODBUS_TEMPLATE_LIST>
inline int8_t Modbus<_MODBUS_TEMPLATE_SPEC>::query(modbus_t telegram)
{
    uint8_t u8regsno, u8bytesno;
    if (this->u8id != 0) return -2;
    if (this->u8state != COM_IDLE) return -1;

    if ((telegram.u8id == 0) || (telegram.u8id > 247)) return -3;

    this->au16regs = telegram.au16reg;

    // telegram header
    this->au8Buffer[ID]     = telegram.u8id;
    this->au8Buffer[FUNC]   = telegram.u8fct;
    this->au8Buffer[ADD_HI] = highByte(telegram.u16RegAdd);
    this->au8Buffer[ADD_LO] = lowByte(telegram.u16RegAdd);

    switch (telegram.u8fct) {
    case MB_FC::MB_FC_READ_COILS:
    case MB_FC::MB_FC_READ_DISCRETE_INPUT:
    case MB_FC::MB_FC_READ_REGISTERS:
    case MB_FC::MB_FC_READ_INPUT_REGISTER:
        this->au8Buffer[NB_HI] = highByte(telegram.u16CoilsNo);
        this->au8Buffer[NB_LO] = lowByte(telegram.u16CoilsNo);
        this->u8BufferSize     = 6;
        break;
    case MB_FC::MB_FC_WRITE_COIL:
        this->au8Buffer[NB_HI] = ((this->au16regs[0] > 0) ? 0xff : 0);
        this->au8Buffer[NB_LO] = 0;
        this->u8BufferSize     = 6;
        break;
    case MB_FC::MB_FC_WRITE_REGISTER:
        this->au8Buffer[NB_HI] = highByte(this->au16regs[0]);
        this->au8Buffer[NB_LO] = lowByte(this->au16regs[0]);
        this->u8BufferSize     = 6;
        break;
    case MB_FC::MB_FC_WRITE_MULTIPLE_COILS:  // TODO: implement "sending coils"
        u8regsno  = telegram.u16CoilsNo / 16;
        u8bytesno = u8regsno * 2;
        if ((telegram.u16CoilsNo % 16) != 0) {
            u8bytesno++;
            u8regsno++;
        }

        this->au8Buffer[NB_HI]    = highByte(telegram.u16CoilsNo);
        this->au8Buffer[NB_LO]    = lowByte(telegram.u16CoilsNo);
        this->au8Buffer[BYTE_CNT] = u8bytesno;
        this->u8BufferSize        = 7;

        for (uint16_t i = 0; i < u8bytesno; i++) {
            if (i % 2) {
                this->au8Buffer[this->u8BufferSize] = lowByte(this->au16regs[i / 2]);
            }
            else {
                this->au8Buffer[this->u8BufferSize] = highByte(this->au16regs[i / 2]);
            }
            this->u8BufferSize++;
        }
        break;

    case MB_FC::MB_FC_WRITE_MULTIPLE_REGISTERS:
        this->au8Buffer[NB_HI]    = highByte(telegram.u16CoilsNo);
        this->au8Buffer[NB_LO]    = lowByte(telegram.u16CoilsNo);
        this->au8Buffer[BYTE_CNT] = (uint8_t)(telegram.u16CoilsNo * 2);
        this->u8BufferSize        = 7;

        for (uint16_t i = 0; i < telegram.u16CoilsNo; i++) {
            this->au8Buffer[this->u8BufferSize] = highByte(this->au16regs[i]);
            this->u8BufferSize++;
            this->au8Buffer[this->u8BufferSize] = lowByte(this->au16regs[i]);
            this->u8BufferSize++;
        }
        break;
    }

    this->sendTxBuffer();
    this->u8state     = COM_WAITING;
    this->u8lastError = 0;
    return 0;
}


/**
 * This method processes functions 1 & 2 (for master)
 * This method puts the slave answer into master data buffer
 *
 * @ingroup register
 * TODO: finish its implementation
 */
template<_MODBUS_TEMPLATE_LIST>
inline void Modbus<_MODBUS_TEMPLATE_SPEC>::get_FC1()
{
    uint8_t u8byte, i;
    u8byte = 3;
    for (i = 0; i < this->au8Buffer[2]; i++) {
        if (i % 2) {  //
            this->au16regs[i / 2] = word(this->au8Buffer[i + u8byte], lowByte(this->au16regs[i / 2]));
        }
        else {
            this->au16regs[i / 2] = word(highByte(this->au16regs[i / 2]), this->au8Buffer[i + u8byte]);
        }
    }
}


/**
 * This method processes functions 3 & 4 (for master)
 * This method puts the slave answer into master data buffer
 *
 * @ingroup register
 */
template<_MODBUS_TEMPLATE_LIST>
inline void Modbus<_MODBUS_TEMPLATE_SPEC>::get_FC3()
{
    uint8_t u8byte, i;
    u8byte = 3;

    for (i = 0; i < this->au8Buffer[2] / 2; i++) {
        this->au16regs[i] = word(this->au8Buffer[u8byte], this->au8Buffer[u8byte + 1]);
        u8byte += 2;
    }
}


/**
 * @brief *** Only for Modbus Master ***
 * This method checks if there is any incoming answer if pending.
 * If there is no answer, it would change Master state to COM_IDLE.
 * This method must be called only at loop section.
 * Avoid any delay() function.
 *
 * Any incoming data would be redirected to this->au16regs pointer,
 * as defined in its modbus_t query telegram.
 *
 * @params	nothing
 * @return errors counter
 * @ingroup loop
 */
template<_MODBUS_TEMPLATE_LIST>
inline int8_t Modbus<_MODBUS_TEMPLATE_SPEC>::poll()
{
    // check if there is any incoming frame
    uint8_t u8current;
    u8current = port->available();


    if ((unsigned long)(millis() - this->u32timeOut) > (unsigned long)(this->u16timeOut)) {
        this->u8state     = COM_IDLE;
        this->u8lastError = NO_REPLY;
        this->u16errCnt++;
        return 0;
    }

    if (u8current == 0) {
        return 0;
    }

    // check T35 after frame end or still no frame end
    if (u8current != this->u8lastRec) {
        this->u8lastRec = u8current;
        this->u32time   = millis();
        return 0;
    }

    if ((unsigned long)(millis() - this->u32time) < (unsigned long)T35) {
        return 0;
    }

    // transfer Serial buffer frame to auBuffer
    this->u8lastRec = 0;
    int8_t i8state  = this->getRxBuffer();
    if (i8state < 6)  // 7 was incorrect for functions 1 and 2 the smallest frame could be 6 bytes long
    {
        this->u8state = COM_IDLE;
        this->u16errCnt++;
        return i8state;
    }

    // validate message: id, CRC, FCT, exception
    uint8_t u8exception = this->validateAnswer();
    if (u8exception != 0) {
        this->u8state = COM_IDLE;
        return u8exception;
    }

    // process answer
    switch (this->au8Buffer[FUNC]) {
    case MB_FC::MB_FC_READ_COILS:
    case MB_FC::MB_FC_READ_DISCRETE_INPUT:
        // call get_FC1 to transfer the incoming message to this->au16regs buffer
        this->get_FC1();
        break;
    case MB_FC::MB_FC_READ_INPUT_REGISTER:
    case MB_FC::MB_FC_READ_REGISTERS:
        // call get_FC3 to transfer the incoming message to this->au16regs buffer
        this->get_FC3();
        break;
    case MB_FC::MB_FC_WRITE_COIL:
    case MB_FC::MB_FC_WRITE_REGISTER:
    case MB_FC::MB_FC_WRITE_MULTIPLE_COILS:
    case MB_FC::MB_FC_WRITE_MULTIPLE_REGISTERS:
        // nothing to do
        break;
    default:
        break;
    }
    this->u8state = COM_IDLE;
    return this->u8BufferSize;
}


}  // namespace modbus_rtu


#endif /* MODBUSRTU_MASTER_H_ */
