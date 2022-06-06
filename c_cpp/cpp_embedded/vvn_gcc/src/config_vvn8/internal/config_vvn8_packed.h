/*
 * config_vvn8_packed.h
 *
 * Created: 09.04.2022 5:08:47
 *  Author: artem
 */ 


#ifndef CONFIG_VVN8_PACKED_H_
#define CONFIG_VVN8_PACKED_H_

#include "config_meas_common.h"


namespace config {

/// Упакованная конфигурация устройства
class ConfigVVN8_packed {
  public:
    /// Тип упакованной калибровки токового входа (температуры и плотности) и выхода (вязкости)
    struct CalibrationItemP {
        /// Значение калибровки для 4х миллиампер
        int32_t value_for_4ma;
        /// Значение калибровки для 20 миллиампер
        int32_t value_for_20ma;
        /// Токовая юстировка
        int32_t current_alignment;
        /// Включить или выключить датчик
        bool enable_sensor;
    } __attribute__((packed));

    // Тип упакованных настроек верхней и нижней уставки
    struct SetpointConfig {
        /// Уровень срабатывания реле уставки
        int32_t lewel_of_relay_activate;

        /// Обработчик срабатывания реле уставки включен
        bool setpoint_enabled;

        /// Звуковой сигнал включен
        bool sound_signal_enabled;

        /// Тип обработчика:
        /// false - сквозной, сигнализация срабатывает при уходе показаний вязкости за пределы уставки, и отключается при возврате в заданный диапазон
        /// true  - запоминающий, сигнализация срабатывает при уходе показаний вязкости за пределы уставки, и остается во включенном состоянии до момента нажатия любой из кнопок.
        bool is_memory_handler;
    } __attribute__((packed));

    /// Настройки нижней уставки
    SetpointConfig low_setpoint{0, false, false, false};

    /// Настройки верхней уставки
    SetpointConfig high_setpoint{0, false, false, false};

    // Настройка выходного ЦАП
    CalibrationItemP config_output_dac;

    // Настройка датчика температуры
    // Функции F -> F -> F -> F ENT Калибровка | АЦП температуры -> 4ma -0.900°C | 20ma 200.00°C
    CalibrationItemP config_temperature_sensor{adc::pack_temp(0.9), adc::pack_temp(200.0), 0, true};

    // Настройка датчика плотности
    // Функции F -> F -> F -> F ENT Калибровка | АЦП плотности -> 4mA 1.000кг/м3 | 20mA 93.00 кг/м3  (3 - индексом)
    CalibrationItemP config_density_sensor{adc::pack_dens(1.0), adc::pack_dens(93.0), 0, true};

    // Тип устройства ВВН8
    VVN_TYPE vvn_type = VVN_TYPE::VVN8_011;

    /// Значение плотности при выключенном датчике - default = 1.000 кг / м3.
    /// При плотности например 920 кг/м3 - на экране значение 0.920 кг / м3 
	/// (точкой отделены три последние значащие цифры
    int32_t default_density;

    /// Включение термокомпенсации ??
    bool enable_termocompensation = true;

    /// Включение индикации вычисления плотности из константной
    bool enable_constant_dencity = true;

    // Частота звука Гц
    uint8_t sound_level = 2;

    // Период архивации измеренной вязкости (Время зап. (записи)) - в секундах
    LoggingTimeout time_interval_of_logging = LoggingTimeout::SEC_5;


    void save();
    void load();
} __attribute__((packed));


}



#endif /* CONFIG_VVN8_PACKED_H_ */