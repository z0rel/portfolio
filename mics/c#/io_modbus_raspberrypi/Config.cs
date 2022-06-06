using System;
using System.Collections.Generic;
using YamlDotNet.Serialization;
using YamlDotNet.Serialization.NamingConventions;

namespace IO1516.src
{

    public class InputRegister
    {
        public int cnc_address { get; set; }
        public int modbus_slave_address { get; set; }
        public Dictionary<int, int> mappings { get; set; }
    }
    public class OutputRegister 
    {
        public int cnc_address { get; set; }
        public int spi_word_num { get; set; }
        public Dictionary<int, int> mappings { get; set; }
    }

    class AppConfig
    {
        public List<InputRegister> input { get; set; }
        public List<OutputRegister> output { get; set; }
        public string serial_port_path { get; set; }
        public string ip_local { get; set; }
        public string ip_cnc { get; set; }
        public int port_cnc { get; set; }
        public int modbus_read_delay_msec { get; set; }
        public int tcp_read_delay_msec { get; set; }
        public int cnc_input_address_start { get; set; }
        public int cnc_input_address_area_length { get; set; }
        public int cnc_output_address_start { get; set; }
        public int cnc_output_address_area_length { get; set; }
        public int modbus_module_delay_msec { get; set; }
        public int serial_port_read_timeout_msec { get; set; }
        public int serial_port_write_timeout_msec { get; set; }
        public int tcp_reconnection_delay { get; set; }
        public int write_spi_vaues_delay_msec { get; set; }
        public int spi_channel_frequency { get; set; }
        public bool verbose_mode { get; set; }
        public int cnc_state_input_registers_address { get; set; }
        public int modbus_inactive_modules_state_renew_delay_msec { get; set; }
    }

    class ExtendedAppConfig
    {
        public AppConfig cfg;
        public Dictionary<int, OutputRegister> out_register_num_to_object;
        public InputRegister[] input;

        public int spi_message_length;

        public ExtendedAppConfig(ref AppConfig _cfg)
        {
            cfg = _cfg;
            out_register_num_to_object = new Dictionary<int, OutputRegister>();
            spi_message_length = 0;
            foreach (var out_register in cfg.output)
            {
                out_register_num_to_object[out_register.cnc_address - cfg.cnc_output_address_start] = out_register;
                if (out_register.spi_word_num > spi_message_length)
                {
                    spi_message_length = out_register.spi_word_num;
                }
            }
            ++spi_message_length; // Учесть нулевой элемент в длине
            if ((spi_message_length % 2) != 0)
            {
                ++spi_message_length; // Выравнивание по 32 битам (двойному слову)
            }
            input = cfg.input.ToArray();
        }

        public void ConsoleWriteLine(string value)
        {
            if (cfg.verbose_mode)
            {
                Console.WriteLine(value);
            }
        }

    }

    class ReadYamlConfig
    {
        public static ExtendedAppConfig read_config(string file_path)
        {

            string yml = System.IO.File.ReadAllText(file_path);
            var deserializer = new DeserializerBuilder()
                .WithNamingConvention(UnderscoredNamingConvention.Instance)  // see height_in_inches in sample yml 
                .Build();

            //yml contains a string containing your YAML
            AppConfig p = deserializer.Deserialize<AppConfig>(yml);

            ExtendedAppConfig v = new ExtendedAppConfig(ref p);
            return v;
        }
    }
}
