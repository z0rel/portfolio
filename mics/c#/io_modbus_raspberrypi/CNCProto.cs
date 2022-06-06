using System;
using System.IO.Ports;
using System.Threading.Tasks;
using NModbus;
using NModbus.Serial;
using CNCNetLib;
using Unosquare.RaspberryIO;
using Unosquare.RaspberryIO.Abstractions;
using Unosquare.WiringPi;



namespace IO1516.src
{
    class CNCProto
    {
        CNCInfoClass CNCInfo;
        private AppConfig cfg;
        private ExtendedAppConfig extcfg;
        private State state;
        private byte[] buf_write_tcp_input_registers;
        private byte[] buf_write_tcp_input_registers_state;

        // Флаг "регистр нужно обновить"
        const int BIT_OF_UPDATE_MASK = Constants.BIT_OF_UPDATE_MASK;
        const int BIT_OF_UPDATE_MASK_INV = Constants.BIT_OF_UPDATE_MASK_INV;

        public CNCProto(ref ExtendedAppConfig _cfg, ref State _state)
        {
            CNCInfo = new CNCInfoClass();
            cfg = _cfg.cfg;
            extcfg = _cfg;
            state = _state;
            buf_write_tcp_input_registers = new byte[cfg.cnc_input_address_area_length * 2];
            buf_write_tcp_input_registers_state = new byte[2]; 
        }


        /// <summary>
        /// connect or disconnect to CNC
        /// </summary>
        public bool connect(bool need_connect = true)
        {
            if (need_connect)
            {
                if (CNCInfo.SetConnectInfo(cfg.ip_local, cfg.ip_cnc, cfg.port_cnc) >= (int)APIReturnCode.API_RET_SUCCESS)
                {
                    if (CNCInfo.Connect() >= (int)APIReturnCode.API_RET_SUCCESS)
                    {
                        extcfg.ConsoleWriteLine("Connected!");
                        return true;
                    }
                    else
                    {
                        extcfg.ConsoleWriteLine("Disconnected!");
                        return false;
                    }
                }
            }
            else
            {
                CNCInfo.Disconnect();
                if (CNCInfo.IsConnect() == false)
                {
                    extcfg.ConsoleWriteLine("Disconnected!");
                    return false;
                }
                else
                {
                    return true;
                }
            }
            return false;
        }

        /// <summary>
        ///  Чтение выходных регистров из стойки УЧПУ
        /// </summary>
        /// <returns></returns>
        private async Task<int> read_out_d_registers()
        {
            const uint devType = 0x08;
            ICNCInfo.ReadedPlcAddrState ret = await CNCInfo.READ_PLC_ADDR(devType, (uint)cfg.cnc_output_address_start, (uint)cfg.cnc_output_address_area_length);

            if (ret.status >= 0)
            {
                for (int i = 0, cnt = 0; i < ret.length; i += 2, cnt++)
                {
                    UInt16 ushort_value = BitConverter.ToUInt16(ret.RetArray, i);
                    int int_value = ushort_value;


                    OutputRegister out_cfg = extcfg.out_register_num_to_object[cnt];

                    int new_value = 0xFFFF;

                    Register16 output_register16 = state.output_device[cnt];
                    foreach (var entry in out_cfg.mappings)
                    {
                        int bitmask = 1 << entry.Key;
                        int new_bit_value = int_value & bitmask;

                        // if ((output_register16.v & bitmask) != new_bit_value)
                        // {
                        //  Console.WriteLine($"{entry.Key}: {Constants.OUTPUT_PIN_NUMBERS[entry.Value]} int_value : {Convert.ToString(int_value, 2)}, new_bit_value: {Convert.ToString(new_bit_value, 2)}, output_register16.v: {Convert.ToString(output_register16.v, 2)}");
                        if (new_bit_value != 0)
                        {

                            //        Console.WriteLine($"pin {Constants.OUTPUT_PIN_NUMBERS[entry.Value]} LOW");
                            Pi.Gpio[Constants.OUTPUT_PIN_NUMBERS[entry.Value]].Write(GpioPinValue.Low);
                            output_register16.v |= bitmask;
                        }
                        else
                        {
                            //        Console.WriteLine($"pin {Constants.OUTPUT_PIN_NUMBERS[entry.Value]} HIGH");
                            Pi.Gpio[Constants.OUTPUT_PIN_NUMBERS[entry.Value]].Write(GpioPinValue.High);
                            output_register16.v &= ~bitmask;
                        }
                        //  Console.WriteLine($"UPD: {entry.Key}: {Constants.OUTPUT_PIN_NUMBERS[entry.Value]} int_value : {Convert.ToString(int_value, 2)}, new_bit_value: {Convert.ToString(new_bit_value, 2)}, output_register16.v: {Convert.ToString(output_register16.v, 2)}");
                        // }
                    }

                    if (cfg.verbose_mode)
                    {
                        extcfg.ConsoleWriteLine($"read_out_d_registers: current out d register: {out_cfg.cnc_address} : {new_value} {Convert.ToString((int)new_value, 2)}  src: {Convert.ToString((int)ushort_value, 2)} register16: {Convert.ToString((int)output_register16.v, 2)}");
                    }
                }
            }
            return 0;
        }

        /// <summary>
        ///  Задача записи состояния входных регистров в стойку УЧПУ
        /// </summary>
        /// <returns></returns>
        private async Task<int> write_in_d_registers()
        {
            uint DevType = 0x08;
            int plc_addr_bufsize = await CNCInfo.READ_PLC_ADDR_BUFFSIZE(DevType, (uint)cfg.cnc_input_address_start, (uint)cfg.cnc_input_address_area_length);
            if (plc_addr_bufsize > 0)
            {
                bool need_write = false;
                foreach (Register16 in_register in state.input_device)
                {
                    if ((in_register.v & BIT_OF_UPDATE_MASK) != 0)
                    {
                        need_write = true;
                        break;
                    }
                }
                if (need_write)
                {
                    Array.Clear(buf_write_tcp_input_registers, 0, buf_write_tcp_input_registers.Length);

                    for (int i = 0; i < cfg.cnc_input_address_area_length; i++)
                    {
                        int offset = i * 2;
                        int int_value = state.input_device[i].v;
                        if ((int_value & BIT_OF_UPDATE_MASK) != 0)
                        {
                            int_value &= BIT_OF_UPDATE_MASK_INV;
                            state.input_device[i].v = int_value;
                        }
                        UInt16 value = (UInt16)(int_value);
                        byte[] valueByteArray = BitConverter.GetBytes(value);
                        Array.Copy(valueByteArray, 0, buf_write_tcp_input_registers, offset, valueByteArray.Length);
                    }
                    if (buf_write_tcp_input_registers.Length != plc_addr_bufsize)
                    {
                        throw new Exception($"writeDataArray.Length != plc_addr_bufsize: {buf_write_tcp_input_registers.Length} != {plc_addr_bufsize}");
                    }

                    int ret = await CNCInfo.WRITE_PLC_ADDR_NoPerm(DevType, (uint)cfg.cnc_input_address_start, (uint)cfg.cnc_input_address_area_length, buf_write_tcp_input_registers);
                    if (cfg.verbose_mode)
                    {
                        extcfg.ConsoleWriteLine($"write_in_d_registers: Result of writing: {ret}");
                    }
                }
            }
            return 0;
        }

        /// <summary>
        /// Записать состояние входных модулей в регистр 
        /// </summary>
        /// <returns></returns>
        private async Task<int> write_in_d_modules_state()
        {
            uint DevType = 0x08;
            if ((state.input_device_state.v & Constants.BIT_OF_NEED_UPDATE_STATE_INPUT_REGISTER) != 0)
            {
                // Console.WriteLine(Convert.ToString(state.input_device_state.v, 2));
                const uint length_of_registers_area = 1U;
                int plc_addr_bufsize = await CNCInfo.READ_PLC_ADDR_BUFFSIZE(DevType, (uint)cfg.cnc_state_input_registers_address, length_of_registers_area);
                if (plc_addr_bufsize > 0)
                {
                    Array.Clear(buf_write_tcp_input_registers_state, 0, buf_write_tcp_input_registers_state.Length);

                    int int_value = state.input_device_state.v & Constants.BIT_OF_NEED_UPDATE_STATE_INPUT_REGISTER_INV;
                    UInt16 value = (UInt16)(int_value);
                    byte[] valueByteArray = BitConverter.GetBytes(value);
                    int offset = 0;
                    Array.Copy(valueByteArray, 0, buf_write_tcp_input_registers_state, offset, valueByteArray.Length);


                    if (buf_write_tcp_input_registers_state.Length != plc_addr_bufsize)
                    {
                        throw new Exception($"writeDataArray.Length != plc_addr_bufsize: {buf_write_tcp_input_registers_state.Length} != {plc_addr_bufsize}");
                    }

                    int ret = await CNCInfo.WRITE_PLC_ADDR_NoPerm(DevType, (uint)cfg.cnc_state_input_registers_address, 1U, buf_write_tcp_input_registers_state);
                    if (cfg.verbose_mode)
                    {
                        Console.WriteLine($"write_in_d_modules_state: Result of writing: {ret}");
                    }
                    state.input_device_state.v &= Constants.BIT_OF_NEED_UPDATE_STATE_INPUT_REGISTER_INV;

                }
            }
            return 0;
        }



        public async Task<int> tcp_read_write_registers()
        {
            if (!CNCInfo.IsConnect())
            {
                connect();
            }
            if (CNCInfo.IsConnect())
            {
                await read_out_d_registers();
                await write_in_d_registers();
                await write_in_d_modules_state();
                await Task.Delay(cfg.tcp_read_delay_msec);
            }
            else
            {
                await Task.Delay(cfg.tcp_reconnection_delay);
            }
            return 0;
        }

        public void init_gpio()
        {
            Pi.Init<BootstrapWiringPi>();
            foreach (int pin_number in Constants.OUTPUT_PIN_NUMBERS)
            {
                var sync_out_pin = Pi.Gpio[pin_number];
                sync_out_pin.PinMode = GpioPinDriveMode.Output;
                sync_out_pin.Write(GpioPinValue.High);
            }
        }

    }
}
