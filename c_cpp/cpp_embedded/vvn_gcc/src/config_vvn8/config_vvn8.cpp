/*
 * config_vvn8.cpp
 *
 * Created: 14.03.2022 11:31:53
 *  Author: artem
 */


#include <avr/eeprom.h>

#include "../archive_item.h"
#include "../types_dictionary.h"


static_assert(EEPROM_FIRST_INITIALIZATION_KEY != 0, "EEPROM_KEY must not be 0");
static_assert(EEPROM_FIRST_INITIALIZATION_KEY != 127, "EEPROM_KEY must not be 127");
static_assert(EEPROM_FIRST_INITIALIZATION_KEY != 255, "EEPROM_KEY must not be 255");


config::UART_Baudrate EEMEM uart_baudrate; //  = UART_Baudrate.B_9600;
config::UART_Parity EEMEM uart_parity; // = UART_Parity.NONE;
config::UART_Stopbits EEMEM uart_stop_bits; // = 1;
config::UART_DataBits EEMEM uart_data_bits;


uint8_t EEMEM uart_modbus_number;
uint8_t EEMEM e_interpolation_points[sizeof(config::InterpolationConfig::interpolationPoints)];
int8_t EEMEM e_password_believer[sizeof(config::PasswordsConfig::password_believer)];
int8_t EEMEM e_password_full[sizeof(config::PasswordsConfig::password_full)];
int8_t EEMEM e_password_master[sizeof(config::PasswordsConfig::password_master)];
int8_t EEMEM e_config_vvn8[sizeof(config::ConfigVVN8_packed)];


static void copy_setpoint(config::ConfigVVN8_packed::SetpointConfig &dst, const config::ConfigVVN8::SetpointConfig &src)
{
    dst.is_memory_handler       = src.is_memory_handler;
    dst.sound_signal_enabled    = src.sound_signal_enabled;
    dst.setpoint_enabled        = src.setpoint_enabled;
    dst.lewel_of_relay_activate = adc::pack_viscosity(src.lewel_of_relay_activate);
}


static void copy_setpoint(config::ConfigVVN8::SetpointConfig &dst, const config::ConfigVVN8_packed::SetpointConfig &src)
{
    dst.is_memory_handler       = src.is_memory_handler;
    dst.sound_signal_enabled    = src.sound_signal_enabled;
    dst.setpoint_enabled        = src.setpoint_enabled;
    dst.lewel_of_relay_activate = adc::unpack_viscosity(src.lewel_of_relay_activate);
}


typedef int32_t (*t_pack_fun)(double value);
typedef double (*t_unpack_fun)(int32_t value);


static void copy_calibration(config::ConfigVVN8::CalibrationItem &dst, const config::ConfigVVN8_packed::CalibrationItemP &src, t_unpack_fun unpack_fun)
{
    dst.interpolation_4_20ma = interpolation::Linear4_20ma((*unpack_fun)(src.value_for_4ma), (*unpack_fun)(src.value_for_20ma));
    dst.current_alignment    = adc::unpack_current32(src.current_alignment);
    dst.enable_sensor        = src.enable_sensor;
}


static void copy_calibration(config::ConfigVVN8_packed::CalibrationItemP &dst, const config::ConfigVVN8::CalibrationItem &src, t_pack_fun pack_fun)
{
    dst.value_for_4ma     = (*pack_fun)(src.interpolation_4_20ma.get_y0());
    dst.value_for_20ma    = (*pack_fun)(src.interpolation_4_20ma.get_y1());
    dst.current_alignment = adc::pack_current32(src.current_alignment);
    dst.enable_sensor     = src.enable_sensor;
}


void config::ConfigVVN8::load()
{
    config::ConfigVVN8_packed cfg;
    cfg.load();

    copy_setpoint(this->low_setpoint, cfg.low_setpoint);
    copy_setpoint(this->high_setpoint, cfg.high_setpoint);
    copy_calibration(this->config_output_dac, cfg.config_output_dac, &adc::unpack_viscosity);
    copy_calibration(this->config_temperature_sensor, cfg.config_temperature_sensor, &adc::unpack_temp);
    copy_calibration(this->config_density_sensor, cfg.config_density_sensor, &adc::unpack_dens);
    this->vvn_type                 = cfg.vvn_type;
    this->default_density          = adc::unpack_dens(cfg.default_density);
    this->enable_termocompensation = cfg.enable_termocompensation;
    this->enable_constant_dencity  = cfg.enable_constant_dencity;
    this->sound_level              = cfg.sound_level;
    this->time_interval_of_logging = cfg.time_interval_of_logging;


    config::InterpolationConfig interp_cfg;
    interp_cfg.load();
    interpolation::InterpolationPoint points[static_cast<uint8_t>(InterpolationPointsSize::value)];

    for (uint8_t i = 0; i < static_cast<uint8_t>(InterpolationPointsSize::value); ++i) {
        points[i].x = adc::pack_voltage32(this->viscosity_interp.get_x(i));
        points[i].y = adc::pack_voltage32(this->viscosity_interp.get_y(i));
    }
    this->viscosity_interp = config::ConfigVVN8::Interpolator(points);
}


void config::ConfigVVN8::save()
{
    config::ConfigVVN8_packed cfg;

    copy_setpoint(cfg.low_setpoint, this->low_setpoint);
    copy_setpoint(cfg.high_setpoint, this->high_setpoint);
    copy_calibration(cfg.config_output_dac, this->config_output_dac, &adc::pack_viscosity);
    copy_calibration(cfg.config_temperature_sensor, this->config_temperature_sensor, &adc::pack_temp);
    copy_calibration(cfg.config_density_sensor, this->config_density_sensor, &adc::pack_dens);
    this->default_density          = adc::pack_dens(cfg.default_density);
    this->enable_termocompensation = cfg.enable_termocompensation;
    this->enable_constant_dencity  = cfg.enable_constant_dencity;
    this->sound_level              = cfg.sound_level;
    this->time_interval_of_logging = cfg.time_interval_of_logging;
    cfg.save();

    config::InterpolationConfig interp_cfg;
    for (uint8_t i = 0; i < static_cast<uint8_t>(InterpolationPointsSize::value); ++i) {
        interp_cfg.interpolationPoints[i].adc_voltage_value         = adc::unpack_voltage32(interp_cfg.interpolationPoints[i].adc_voltage_value);
        interp_cfg.interpolationPoints[i].kinematic_viscosity_value = adc::unpack_voltage32(interp_cfg.interpolationPoints[i].kinematic_viscosity_value);
    }
    interp_cfg.save();
}


void config::ConfigVVN8_packed::load() { eeprom_read_block(e_config_vvn8, reinterpret_cast<uint8_t *>(this), sizeof(config::ConfigVVN8_packed)); }


void config::ConfigVVN8_packed::save() { eeprom_update_block(reinterpret_cast<uint8_t *>(this), e_config_vvn8, sizeof(config::ConfigVVN8_packed)); }


void config::PasswordsConfig::load()
{
    eeprom_read_block(e_password_believer, reinterpret_cast<uint8_t *>(this->password_believer), sizeof(config::PasswordsConfig::password_believer));
    eeprom_read_block(e_password_full, reinterpret_cast<uint8_t *>(this->password_full), sizeof(config::PasswordsConfig::password_full));
    eeprom_read_block(e_password_master, reinterpret_cast<uint8_t *>(this->password_master), sizeof(config::PasswordsConfig::password_master));
}


void config::PasswordsConfig::save()
{
    eeprom_update_block(reinterpret_cast<uint8_t *>(this->password_believer), e_password_believer, sizeof(config::PasswordsConfig::password_believer));
    eeprom_update_block(reinterpret_cast<uint8_t *>(this->password_full), e_password_full, sizeof(config::PasswordsConfig::password_full));
    eeprom_update_block(reinterpret_cast<uint8_t *>(this->password_master), e_password_master, sizeof(config::PasswordsConfig::password_master));
}


void config::InterpolationConfig::load()
{
    eeprom_read_block(e_interpolation_points, reinterpret_cast<uint8_t *>(this->interpolationPoints), sizeof(config::InterpolationConfig::interpolationPoints));
}


void config::InterpolationConfig::save()
{
    eeprom_update_block(reinterpret_cast<uint8_t *>(this->interpolationPoints), e_interpolation_points, sizeof(config::InterpolationConfig::interpolationPoints));
}


void config::first_init_eeprom()
{
    {
        ModbusConfig modbus_config;
        modbus_config.save();
    }
    {
        InterpolationConfig interp_config;
        interp_config.save();
    }
    {
        PasswordsConfig passwords_config;
        passwords_config.save();
    }
    {
        ConfigVVN8_packed config_vvn8;
        config_vvn8.save();
    }

    Archive::ArchiveItem::init_eeprom_array();

    eeprom_update_byte(&eeprom_first_initialization_key, EEPROM_FIRST_INITIALIZATION_KEY);
}

// TODO: можно упаковать биты конфига Modbus и парольных символов, тогда освободится место для 3х записей журнала
void config::ModbusConfig::load()
{
    this->baudrate      = static_cast<UART_Baudrate>(eeprom_read_byte(reinterpret_cast<uint8_t *>(&uart_baudrate)));
    this->parity        = static_cast<UART_Parity>(eeprom_read_byte(reinterpret_cast<uint8_t *>(&uart_parity)));
    this->stopbits      = static_cast<UART_Stopbits>(eeprom_read_byte(reinterpret_cast<uint8_t *>(&uart_stop_bits)));
    this->data_bits     = static_cast<UART_DataBits>(eeprom_read_byte(reinterpret_cast<uint8_t *>(&uart_data_bits)));
    this->device_number = eeprom_read_byte(&uart_modbus_number);
}


void config::ModbusConfig::save()
{
    eeprom_update_byte(reinterpret_cast<uint8_t *>(&uart_baudrate), static_cast<uint8_t>(this->baudrate));
    eeprom_update_byte(reinterpret_cast<uint8_t *>(&uart_parity), static_cast<uint8_t>(this->parity));
    eeprom_update_byte(reinterpret_cast<uint8_t *>(&uart_stop_bits), static_cast<uint8_t>(this->stopbits));
    eeprom_update_byte(reinterpret_cast<uint8_t *>(&uart_data_bits), static_cast<uint8_t>(this->data_bits));
    eeprom_update_byte(reinterpret_cast<uint8_t *>(&uart_modbus_number), static_cast<uint8_t>(this->device_number));
}
