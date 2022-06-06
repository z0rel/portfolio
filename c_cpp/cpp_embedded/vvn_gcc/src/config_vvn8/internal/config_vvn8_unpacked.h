/*
 * config_vvn8_unpacked.h
 *
 * Created: 09.04.2022 5:09:01
 *  Author: artem
 */


#ifndef CONFIG_VVN8_UNPACKED_H_
#define CONFIG_VVN8_UNPACKED_H_

#include "config_meas_common.h"


namespace config {


/// Конфигурация устройства (распакованная)
class ConfigVVN8 {
  public:
    /// Тип калибровки токового входа (температуры и плотности) и выхода (вязкости)
    struct CalibrationItem {
        /// Значение калибровки для 4-20 миллиампер
        interpolation::Linear4_20ma interpolation_4_20ma;
        /// Токовая юстировка
        double current_alignment;
        /// Включить или выключить датчик
        bool enable_sensor;
    };

    // Тип настроек верхней и нижней уставки
    struct SetpointConfig {
        /// Уровень срабатывания реле уставки
        TypeViscosityDynamics lewel_of_relay_activate;

        /// Обработчик срабатывания реле уставки включен
        bool setpoint_enabled;

        /// Звуковой сигнал включен
        bool sound_signal_enabled;

        /// Тип обработчика:
        /// false - сквозной, сигнализация срабатывает при уходе показаний вязкости за пределы уставки, и отключается при возврате в заданный диапазон
        /// true  - запоминающий, сигнализация срабатывает при уходе показаний вязкости за пределы уставки, и остается во включенном состоянии до момента нажатия любой из кнопок.
        bool is_memory_handler;
    };

    /// Настройки нижней уставки
    SetpointConfig low_setpoint{0, false, false, false};

    /// Настройки верхней уставки
    SetpointConfig high_setpoint{0, false, false, false};

    // Настройка выходного ЦАП
    CalibrationItem config_output_dac;

    // Настройка датчика температуры
    // Функции F -> F -> F -> F ENT Калибровка | АЦП температуры -> 4ma -0.900°C | 20ma 200.00°C
    CalibrationItem config_temperature_sensor{{0.9, 200.0 - 0.9}, 0.0, true};

    // Настройка датчика плотности
    // Функции F -> F -> F -> F ENT Калибровка | АЦП плотности -> 4mA 1.000кг/м3 | 20mA 93.00 кг/м3  (3 - индексом)
    CalibrationItem config_density_sensor{{1.0, 93.0 - 1.0}, 0.0, true};

    // Тип устройства ВВН8
    VVN_TYPE vvn_type = VVN_TYPE::VVN8_011;

    /// Значение плотности при выключенном датчике - default = 1.000 кг / м3.
    /// При плотности например 920 кг/м3 - на экране значение 0.920 кг / м3 (точкой отделены три последние значащие цифры
    TypeDensity default_density;

    /// Включение термокомпенсации ??
    bool enable_termocompensation = true;

    /// Включение индикации вычисления плотности из константной
    bool enable_constant_dencity = true;

    // Частота звука Гц
    uint8_t sound_level = 2;

    // Период архивации измеренной вязкости (Время зап. (записи)) - в секундах
    LoggingTimeout time_interval_of_logging = LoggingTimeout::SEC_5;

    /// Тип интерполятора вязкости
    typedef interpolation::CalculatedSpline<static_cast<uint8_t>(InterpolationPointsSize::value)> Interpolator;

    /// Интерполятор вязкости
    Interpolator viscosity_interp;

    void save_interpolation();
    void save();
    void load();

    inline double getMultiplierViscosity() const;
};


inline double ConfigVVN8::getMultiplierViscosity() const
{
    switch (this->vvn_type) {
    case VVN_TYPE::VVN8_011:
        return 1.25;
    case VVN_TYPE::VVN8_021:
        return 12.5;
    case VVN_TYPE::VVN8_031:
        return 125.0;
    case VVN_TYPE::VVN8_041:
        return 1250.0;
    case VVN_TYPE::VVN8_051:
        return 6250.0;
    default:
        return 1.25;
    }
    return 1.25;
}

} // namespace config


#endif /* CONFIG_VVN8_UNPACKED_H_ */
