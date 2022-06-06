/*
 * hmi_state.h
 *
 * Created: 10.03.2022 13:32:36
 *  Author: artem
 */


#ifndef HMI_STATE_H_
#define HMI_STATE_H_


#include "../config_vvn8/config_vvn8.h"
#include "hmi_menu.h"

namespace state {

enum class AccessLevel : uint8_t {
    OPERATOR, // Оператор
    MASTER, // Мастер
    FULL, // Полный
    BELIEVER // Поверитель
};


class HmiStateOutput {
  public:
    char row0[17];
    char row1[17];

    // Уставка звук включен
    bool sound_wawed = false;

    // Уставка верхнего уровня сработала
    bool relay_high_on = false;
    // Уставка нижнего уровня сработала
    bool relay_low_on = false;

    // Светодиод Яркость
    bool led_bright = false;

    // Светодиод "Работа"
    bool now_is_worked = false;

    /// Выходной код ЦАП. Iout = "вязкость" (Па * с * кг/м3) / "Ki" (Па * с * кг/м3 / mA) + 4
    // Ki - Коэффициент преобразования измеряемой величины в выходной сигнал тока, приведенный в таблице 3.1 для конкретного исполнения вяскозиметра
    // | Шифр         | Коэффициент преобразования  измеряемой 
    // | исполнения   | величины в выходной сигнал тока, Кi Па*с*кг/м3
    // | вискозиметра |                                     ----------
    // |              |                                         мА
    // | ------------ | ------------
    // | ВВН-8-011    |    1,25
    // | ВВН-8-021    |   12,5
    // | ВВН-8-031    |  125
    // | ВВН-8-041    | 1250
    // | ВВН-8-051    | 6250


    double dac_value_current = 0.0;

    uint16_t dac_value_code = 0;

    // Тип открытого экрана
    HMI_Menu hmi_menu_item = HMI_Menu::TEST;
};


class HmiStateInput {
  public:
    /// Нажата кнопка "Вниз"
    bool btn_down_pressed = false;

    /// Нажата кнопка "Esc"
    bool btn_esc_pressed = false;

    /// Нажата кнопка "Enter"
    bool btn_enter_pressed = false;

    /// Нажата кнопка "F"
    bool btn_f_pressed = false;

    /// Нажата кнопка "-"
    bool btn_tire_pressed = false;

    /// Температура измеренная - код АЦП
    uint16_t temperature_raw = 0;

    /// Температура измеренная отфильтрованная - код АЦП
    uint16_t temperature_raw_filtered = 0;

    /// Плотность измеренная - код АЦП
    uint16_t dencity_raw = 0;

    /// Плотность измеренная отфильтрованная - код АЦП
    uint16_t dencity_raw_filtered = 0;


    /// Вязкость измеренная - код АЦП
    uint16_t viscosity_raw = 0;

    /// Вязкость измеренная отфильтрованная - код АЦП
    uint16_t viscosity_raw_filtered = 0;

    /// Температура измеренная - значение тока 
    double temperature_current = 0.0;

    /// Температура измеренная
    config::TypeTemperature temperature_value = 0.0;

    /// Плотность измеренная - значение тока
    double density_current = 0.0;

    /// Плотность измеренная в кг / м3
    config::TypeDensity density_value = 0.0;

    /// Вязкость измеренная - значение напряжения 
    double viscosity_voltage = 0.0;

    /// Вязкость измеренная в единицах Па * с * кг / м3 - Кинематическая вязкость
    config::TypeViscosityMeasured viscosity_value_kinematik = 0.0;

    // Динамическая вязкость - Па * с
    config::TypeViscosityMeasured viscosity_value_dynamic = 0.0;

    /// Датчик вязкости подключён или нет
    bool viscosity_sensor_is_connected = true;

    /// Номер отображаемой архивной записи
    uint16_t archive_number = 0;
    /// Максимальный номер архивной записи
    uint16_t archive_max = 0;

    AccessLevel access_level = AccessLevel::OPERATOR;
};



extern HmiStateInput state_input_current;

extern HmiStateOutput state_output_current;
extern HmiStateOutput state_output_previous;


}

#endif /* HMI_STATE_H_ */
