/*
 * ModbusRTU_messages.h
 *
 * Created: 09.04.2022 15:27:02
 *  Author: artem
 */


#ifndef MODBUSRTU_MESSAGES_H_
#define MODBUSRTU_MESSAGES_H_


namespace modbus_rtu {


enum { RESPONSE_SIZE = 6, EXCEPTION_SIZE = 3, CHECKSUM_SIZE = 2 };

/**
 * @enum MESSAGE
 * @brief
 * Indexes to telegram frame positions
 */
enum MESSAGE {
    /// ID field
    ID = 0,
    /// Function code position
    FUNC = 1,
    /// Address high byte
    ADD_HI = 2,
    /// Address low byte
    ADD_LO = 3,
    /// Number of coils or registers high byte
    NB_HI = 4,
    /// Number of coils or registers low byte
    NB_LO = 5,
    /// byte counter
    BYTE_CNT = 6
};

/**
 * @enum MB_FC
 * @brief
 * Modbus function codes summary.
 * These are the implement function codes either for Master or for Slave.
 *
 * @see also modbus_t
 */
enum class MB_FC : uint8_t {
    /// null operator
    MB_FC_NONE = 0,

    /// FCT=1 -> read coils or digital outputs
    MB_FC_READ_COILS = 1,

    /// FCT=2 -> read digital inputs
    MB_FC_READ_DISCRETE_INPUT = 2,

    /// FCT=3 -> read registers or analog outputs
    MB_FC_READ_REGISTERS = 3,

    /// FCT=4 -> read analog inputs
    MB_FC_READ_INPUT_REGISTER = 4,

    /// FCT=5 -> write single coil or output
    MB_FC_WRITE_COIL = 5,

    /// FCT=6 -> write single register
    MB_FC_WRITE_REGISTER = 6,

    /// FCT=15 -> write multiple coils or outputs
    MB_FC_WRITE_MULTIPLE_COILS = 15,

    /// FCT=16 -> write multiple registers
    MB_FC_WRITE_MULTIPLE_REGISTERS = 16
};


enum COM_STATES { COM_IDLE = 0, COM_WAITING = 1 };


enum ERR_LIST { ERR_NOT_MASTER = -1, ERR_POLLING = -2, ERR_BUFF_OVERFLOW = -3, ERR_BAD_CRC = -4, ERR_EXCEPTION = -5 };


enum { NO_REPLY = 255, EXC_FUNC_CODE = 1, EXC_ADDR_RANGE = 2, EXC_REGS_QUANT = 3, EXC_EXECUTE = 4 };


enum class MB_SUPPORTED : uint8_t {
    MB_FC_READ_COILS               = MB_FC::MB_FC_READ_COILS,
    MB_FC_READ_DISCRETE_INPUT      = MB_FC::MB_FC_READ_DISCRETE_INPUT,
    MB_FC_READ_REGISTERS           = MB_FC::MB_FC_READ_REGISTERS,
    MB_FC_READ_INPUT_REGISTER      = MB_FC::MB_FC_READ_INPUT_REGISTER,
    MB_FC_WRITE_COIL               = MB_FC::MB_FC_WRITE_COIL,
    MB_FC_WRITE_REGISTER           = MB_FC::MB_FC_WRITE_REGISTER,
    MB_FC_WRITE_MULTIPLE_COILS     = MB_FC::MB_FC_WRITE_MULTIPLE_COILS,
    MB_FC_WRITE_MULTIPLE_REGISTERS = MB_FC::MB_FC_WRITE_MULTIPLE_REGISTERS
};


#define T35 5


#define MAX_BUFFER 64  //!< maximum size for the communication buffer in bytes


}  // namespace modbus_rtu


#endif /* MODBUSRTU_MESSAGES_H_ */
