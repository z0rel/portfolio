using System;


// Состояние регистров чтения и записи
namespace IO1516.src
{
    internal class Register16
    {
        // Значение регистра
        public int _value;

        public Register16(int val)
        {
            _value = val;
        }

        public int v 
        {
            get
            {
                System.Threading.Interlocked.MemoryBarrier();
                return _value;
            }
            set
            {
                System.Threading.Interlocked.Exchange(ref _value, value);
            }
        }
    }

    internal class Register16_output : Register16
    {
        public readonly int spi_byte_position;

        public Register16_output(int val, int spi_word_num)
            : base(val) 
        {
            spi_byte_position = spi_word_num * 2;
        }

    }

    internal class State
    {
        public Register16[] input_device;
        public Register16 input_device_state;
        public Register16_output[] output_device;

        public State(ref ExtendedAppConfig extcfg)
        {
            input_device_state = new Register16(0);
            input_device = new Register16[extcfg.cfg.cnc_input_address_area_length];
            for (int i = 0; i < input_device.Length; ++i)
            {
                input_device[i] = new Register16(Constants.BIT_OF_UPDATE_MASK);
            }

            output_device = new Register16_output[extcfg.cfg.cnc_output_address_area_length];
            for (int i = 0; i < output_device.Length; ++i)
            {
                output_device[i] = new Register16_output(0xFFFF | Constants.BIT_OF_UPDATE_MASK, extcfg.out_register_num_to_object[i].spi_word_num);
            }
        }
    }
}
