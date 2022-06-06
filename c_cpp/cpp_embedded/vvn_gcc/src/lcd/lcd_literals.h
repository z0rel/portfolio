#ifndef LCD_LITERALS_H
#define LCD_LITERALS_H
#include <avr/pgmspace.h>

#include <stdint.h>

namespace LCD_constants {

    ///    ВВН-8
#   define LENGTH_Logo_vvn 8
    /// ПАО {\"}Автоматика{\"}
#   define LENGTH_Logo_Automatics 16
    ///     Тест ОЗУ
#   define LENGTH_Test_memory 12
    ///  ОЗУ неисправна
#   define LENGTH_Test_memory_error 15
    /// г
#   define LENGTH_g 1
    /// Па{*}с{*}кг/m{^3}
#   define LENGTH_Pa_s_kg_m3 10
    /// Па{*}с
#   define LENGTH_Pa_s 4
    /// Функции
#   define LENGTH_Functions 7
    /// Сброс
#   define LENGTH_Reset 5
    /// Квитирование
#   define LENGTH_Kvitirovaine 12
    /// Очистка архива
#   define LENGTH_Reset_archive 14
    /// {^grad}C
#   define LENGTH_grad_Celsius 2
    /// Плотность
#   define LENGTH_Dencity 9
    /// p=
#   define LENGTH_p_equal 2
    /// кг/м{^3}
#   define LENGTH_kg_div_m3 5
    /// Доступ
#   define LENGTH_Access 6
    /// Открытие доступа
#   define LENGTH_Opening_access 16
    /// Доступ поверит.
#   define LENGTH_Access_of_believer 15
    /// Пароль
#   define LENGTH_Password 6
    /// АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЭЮЯ01234567890_
#   define LENGTH_password_chars 44
    /// Установка пароля
#   define LENGTH_Setting_password 16
    /// Закрыть доступ?
#   define LENGTH_Close_access_query 15
    /// Доступ оператор
#   define LENGTH_Access_of_operator_level 15
    /// Настройка
#   define LENGTH_Setup 9
    /// Установка часов
#   define LENGTH_Set_clocks 15
    /// Дата
#   define LENGTH_Date 4
    /// Время
#   define LENGTH_Time 5
    /// Конфигурация
#   define LENGTH_Configuration 12
    /// Термокомпен. да
#   define LENGTH_Termocompensation_yes 15
    /// Термокомпен. нет
#   define LENGTH_Termocompensation_no 16
    /// Индикация прив.
#   define LENGTH_Indication_reduced 15
    /// Индикация текущ.
#   define LENGTH_Indication_current 16
    /// Плотность измер.
#   define LENGTH_Dencity_measurement 16
    /// Плотность ввод.
#   define LENGTH_Dencity_constant 15
    /// Время зап.
#   define LENGTH_Time_of_writing 10
    /// сек
#   define LENGTH_sec 3
    /// мин
#   define LENGTH_min 3
    /// Записано
#   define LENGTH_Writed 8
    /// Верхний уровень
#   define LENGTH_Top_level 15
    /// Реле норм. выкл.
#   define LENGTH_Relay_normal_off 16
    /// Реле норм. вкл.
#   define LENGTH_Relay_normal_on 15
    /// Канал запмоинает
#   define LENGTH_Chanel_memorize 16
    /// Канал сквозной
#   define LENGTH_Chanel_through 14
    /// Звук нет
#   define LENGTH_Sound_off 8
    /// Звук 1Гц
#   define LENGTH_Sound_1Gz 8
    /// Звук 2Гц
#   define LENGTH_Sound_2Gz 8
    /// Уставка
#   define LENGTH_Setting 7
    /// Нижний уровень
#   define LENGTH_Low_lewel 14
    /// Параметры RS-485
#   define LENGTH_Parameters_of_RS485 16
    /// Сетевой адр.
#   define LENGTH_Network_address 12
    /// Бод
#   define LENGTH_Baud 3
    /// Калибровка
#   define LENGTH_Calibration 10
    /// датчика
#   define LENGTH_Sensor 7
    /// АЦП температуры
#   define LENGTH_ADC_of_temperature 15
    /// мА
#   define LENGTH_mA 2
    /// АЦП плотности
#   define LENGTH_ADC_of_dencity 13
    /// Юстировка
#   define LENGTH_Alignment 9
    /// ЦАП вязкости
#   define LENGTH_DAC_of_viscosity 12
    /// Диапазон
#   define LENGTH_Range 8
    /// ВВН-8-011
#   define LENGTH_VVN_8_011 9
    /// ВВН-8-021
#   define LENGTH_VVN_8_021 9
    /// ВВН-8-031
#   define LENGTH_VVN_8_031 9
    /// ВВН-8-041
#   define LENGTH_VVN_8_041 9
    /// ВВН-8-051
#   define LENGTH_VVN_8_051 9
    /// Uацп
#   define LENGTH_Uadc 4
}

#endif