using System;
using System.Threading;
using System.Threading.Tasks;
using System.IO.Ports;
using NModbus;
using NModbus.Serial;


namespace IO1516.src
{
    class ModbusHandleInputModules
    {
        private SerialPort serial_modbus_port;
        private IModbusMaster modbus_master;

        private AppConfig cfg;
        private ExtendedAppConfig extcfg;
        private State state;
        private SemaphoreSlim semaphore;

        public ModbusHandleInputModules(ref ExtendedAppConfig _cfg, ref State _state)
        {
            cfg = _cfg.cfg;
            extcfg = _cfg;
            state = _state;
            semaphore = new SemaphoreSlim(1, 1);

            foreach (InputRegister val in cfg.input)
            {
                module_active_state_set(val.modbus_slave_address - 1);
            }
        }


        public void open_serial_port()
        {
            serial_modbus_port = new SerialPort(cfg.serial_port_path);
            serial_modbus_port.BaudRate = 38400;
            serial_modbus_port.DataBits = 8;
            serial_modbus_port.Parity = Parity.Even;
            serial_modbus_port.StopBits = StopBits.One;
            serial_modbus_port.Open();
            serial_modbus_port.ReadTimeout = cfg.serial_port_read_timeout_msec;
            serial_modbus_port.WriteTimeout = cfg.serial_port_write_timeout_msec;
            modbus_master = (new ModbusFactory()).CreateRtuMaster(serial_modbus_port);
        }


        /// <summary>
        /// Прочитать состояние модуля
        /// </summary>
        private async Task<bool> modbus_read_module_values(InputRegister input_register_cfg)
        {
            const UInt16 startAddress = 0;
            const UInt16 numberOfPoints = 0x10;
            // write three registers

            await semaphore.WaitAsync();
            bool[] inputs = await modbus_master.ReadInputsAsync((byte)input_register_cfg.modbus_slave_address, startAddress, numberOfPoints);

            int new_value = 0;

            foreach (var entry in input_register_cfg.mappings)
            {
                bool input_value = inputs[entry.Key];
                if (input_value)
                {
                    new_value |= 1 << entry.Value;
                }
            }
            Register16 input_register16 = state.input_device[input_register_cfg.cnc_address - cfg.cnc_input_address_start];
            if ((input_register16.v & Constants.BIT_OF_UPDATE_MASK_INV) != new_value)
            {
                input_register16.v = new_value | Constants.BIT_OF_UPDATE_MASK;
                if (cfg.verbose_mode)
                {
                    extcfg.ConsoleWriteLine($"modbus_read_registers: new input: {input_register_cfg.modbus_slave_address} New Value: {Convert.ToString((int)input_register16.v, 2)}");
                }
            }
            if (cfg.verbose_mode)
            {
                extcfg.ConsoleWriteLine($"modbus_read_registers: input: {input_register_cfg.modbus_slave_address} New Value: {Convert.ToString((int)input_register16.v, 2)}");
            }

            semaphore.Release();
            return true;
        }

        private void module_active_state_clear(int module_number)
        {
            state.input_device_state.v &= ~(1 << module_number);
            state.input_device_state.v |= Constants.BIT_OF_NEED_UPDATE_STATE_INPUT_REGISTER;
        }
        private void module_active_state_set(int module_number)
        {
            state.input_device_state.v |= ((1 << module_number) | Constants.BIT_OF_NEED_UPDATE_STATE_INPUT_REGISTER);
        }
        private bool module_is_inactive(int module_number)
        {
            return (state.input_device_state.v & (1 << module_number)) == 0;
        }
        private bool module_is_active(int module_number)
        {
            return (state.input_device_state.v & (1 << module_number)) != 0;
        }

        /// <summary>
        ///     Задача чтения входных регистров из модулей
        /// </summary>
        public async Task<int> modbus_read_registers()
        {
            foreach (InputRegister val in cfg.input)
            {
                int module_number = -1;
                bool semaphore_is_released = true;
                try
                {
                    module_number = val.modbus_slave_address - 1;

                    // Быстрый пропуск всех неактивных модулей - для избегания задержки опроса. Состояние неактивных модулей опрашивается в отдельной задаче
                    if (module_is_inactive(module_number))
                    {
                        if (cfg.verbose_mode)
                        {
                            extcfg.ConsoleWriteLine($"modbus_read_registers: Module {module_number} is not active now, state: {Convert.ToString((int)state.input_device_state.v, 2)}");
                        }
                        continue;
                    }

                    semaphore_is_released = false;
                    semaphore_is_released = await modbus_read_module_values(val);

                    // Обновить бит состояния если вдруг он стал неактивным
                    if (module_is_inactive(module_number))
                    {
                        if (cfg.verbose_mode)
                        {
                            extcfg.ConsoleWriteLine($"modbus_read_registers: Module {module_number} is deactivated now");
                        }
                        module_active_state_clear(module_number);
                    }

                    await Task.Delay(cfg.modbus_module_delay_msec);
                }
                catch (System.TimeoutException)
                {
                    if (!semaphore_is_released)
                    {
                        semaphore.Release();
                    }
                    if (cfg.verbose_mode)
                    {
                        extcfg.ConsoleWriteLine($"modbus_read_registers: Error: Module {module_number} is not available");
                    }
                    module_active_state_clear(module_number);
                }
                catch
                {
                    if (!semaphore_is_released)
                    {
                        semaphore.Release();
                    }
                    if (cfg.verbose_mode)
                    {
                        extcfg.ConsoleWriteLine($"modbus_read_registers: Error: Unknown Modbus Error, module {module_number}");
                    }
                    module_active_state_clear(module_number);
                }
                await Task.Delay(cfg.modbus_read_delay_msec);
            }
            return 0;
        }

        /// <summary>
        ///  Перечитать состояние неактивных модулей
        /// </summary>
        /// <returns></returns>
        public async Task<int> modbus_read_bad_modules_state()
        {
            foreach (InputRegister val in cfg.input)
            {
                int module_number = -1;
                bool semaphore_is_released = true;
                if (module_is_inactive(module_number))
                {
                    try
                    {
                        module_number = val.modbus_slave_address - 1;

                        // Опрос всех неактивных модулей 
                        semaphore_is_released = false;
                        semaphore_is_released = await modbus_read_module_values(val);
                        module_active_state_set(module_number);
                        if (cfg.verbose_mode)
                        {
                            extcfg.ConsoleWriteLine($"modbus_read_bad_modules_state: Module {module_number} is activated now");
                        }
                    }
                    catch (System.TimeoutException)
                    {
                        if (!semaphore_is_released)
                        {
                            semaphore.Release();
                        }
                        if (cfg.verbose_mode)
                        {
                            extcfg.ConsoleWriteLine($"modbus_read_bad_modules_state: Error: Module {module_number} is not available");
                        }
                        if (module_is_active(module_number))
                        {
                            module_active_state_clear(module_number);
                        }
                    }
                    catch
                    {
                        if (!semaphore_is_released)
                        {
                            semaphore.Release();
                        }
                        if (cfg.verbose_mode)
                        {
                            extcfg.ConsoleWriteLine($"modbus_read_bad_modules_state: Error: Unknown Modbus Error, module {module_number}");
                        }
                        if (module_is_active(module_number))
                        {
                            module_active_state_clear(module_number);
                        }
                    }
                    await Task.Delay(cfg.modbus_read_delay_msec);
                }
            }
            await Task.Delay(cfg.modbus_inactive_modules_state_renew_delay_msec);
            return 0;
        }
    }
}
