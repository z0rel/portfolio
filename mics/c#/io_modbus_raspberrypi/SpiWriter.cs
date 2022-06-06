using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using CNCNetLib;
using Unosquare.RaspberryIO;
using Unosquare.RaspberryIO.Abstractions;
using Unosquare.WiringPi;

namespace IO1516.src
{
    class SpiWriter
    {
        private AppConfig cfg;
        private ExtendedAppConfig extcfg;
        private State state;

        private byte[] buf_spi0;
        
        public SpiWriter(ref ExtendedAppConfig _cfg, ref State _state)
        {
            cfg = _cfg.cfg;
            extcfg = _cfg;
            state = _state;
            buf_spi0 = new byte[extcfg.spi_message_length * 2];
            if (cfg.verbose_mode)
            {
                extcfg.ConsoleWriteLine($"SPI message length: {extcfg.spi_message_length}");
            }
        }

        public void init_gpio()
        {
            Pi.Init<BootstrapWiringPi>();
            var sync_out_pin = Pi.Gpio[27];
            sync_out_pin.PinMode = GpioPinDriveMode.Output;
            sync_out_pin.Write(GpioPinValue.High);
            Pi.Spi.Channel0Frequency = cfg.spi_channel_frequency;
        }

        public async Task<int> write_spi_values()
        {
            bool need_update = false;
            foreach (Register16 out_device in state.output_device)
            {
                if ((out_device.v & Constants.BIT_OF_UPDATE_MASK) != 0)
                {
                    need_update = true;
                    out_device.v &= Constants.BIT_OF_UPDATE_MASK_INV;
                }
            }

            if (need_update)
            {
                Array.Clear((Array)buf_spi0, 0, buf_spi0.Length);
                UInt16 value;
                foreach (Register16_output out_device in state.output_device)
                {
                    value = (UInt16)(int)(out_device.v & Constants.BIT_OF_UPDATE_MASK_INV);
                    buf_spi0[out_device.spi_byte_position] = (byte)(value & 0xFF);
                    buf_spi0[out_device.spi_byte_position + 1] = (byte)(value >> 8);
                }
                var sync_out_pin = Pi.Gpio[Constants.PIN_UPDATE_SHIFT_REGISTER_OUT_VALUES];
                sync_out_pin.Write(GpioPinValue.Low);
                Pi.Spi.Channel0.SendReceive(buf_spi0);
                sync_out_pin.Write(GpioPinValue.High);
            }

            await Task.Delay(cfg.tcp_read_delay_msec);
            return 0;
        }
    }
}
