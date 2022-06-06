#include <avr/pgmspace.h>

#include <stdint.h>

namespace LCD_constants {

    ///    ВВН-8
    const char Logo_vvn[] PROGMEM = "\x20\x20\x20\x42\x42\x48\x2D\x38";
    /// ПАО {\"}Автоматика{\"}
    const char Logo_Automatics[] PROGMEM = "\xA8\x41\x4F\x20\x22\x41\xB3\xBF\x6F\xBC\x61\xBF\xB8\xBA\x61\x22";
    ///     Тест ОЗУ
    const char Test_memory[] PROGMEM = "\x20\x20\x20\x20\x54\x65\x63\xBF\x20\x4F\xA4\xA9";
    ///  ОЗУ неисправна
    const char Test_memory_error[] PROGMEM = "\x20\x4F\xA4\xA9\x20\xBD\x65\xB8\x63\xBE\x70\x61\xB3\xBD\x61";
    /// г
    const char g[] PROGMEM = "\xB4";
    /// Па{*}с{*}кг/m{^3}
    const char Pa_s_kg_m3[] PROGMEM = "\xA8\x61\x6\x63\x6\xBA\xB4\x2F\x6D\x3";
    /// Па{*}с
    const char Pa_s[] PROGMEM = "\xA8\x61\x6\x63";
    /// Функции
    const char Functions[] PROGMEM = "\xAA\x79\xBD\xBA\xE5\xB8\xB8";
    /// Сброс
    const char Reset[] PROGMEM = "\x43\xB2\x70\x6F\x63";
    /// Квитирование
    const char Kvitirovaine[] PROGMEM = "\x4B\xB3\xB8\xBF\xB8\x70\x6F\xB3\x61\xBD\xB8\x65";
    /// Очистка архива
    const char Reset_archive[] PROGMEM = "\x4F\xC0\xB8\x63\xBF\xBA\x61\x20\x61\x70\x78\xB8\xB3\x61";
    /// {^grad}C
    const char grad_Celsius[] PROGMEM = "\x7\x43";
    /// Плотность
    const char Dencity[] PROGMEM = "\xA8\xBB\x6F\xBF\xBD\x6F\x63\xBF\xC4";
    /// p=
    const char p_equal[] PROGMEM = "\x70\x3D";
    /// кг/м{^3}
    const char kg_div_m3[] PROGMEM = "\xBA\xB4\x2F\xBC\x3";
    /// Доступ
    const char Access[] PROGMEM = "\xE0\x6F\x63\xBF\x79\xBE";
    /// Открытие доступа
    const char Opening_access[] PROGMEM = "\x4F\xBF\xBA\x70\xC3\xBF\xB8\x65\x20\xE3\x6F\x63\xBF\x79\xBE\x61";
    /// Доступ поверит.
    const char Access_of_believer[] PROGMEM = "\xE0\x6F\x63\xBF\x79\xBE\x20\xBE\x6F\xB3\x65\x70\xB8\xBF\x2E";
    /// Пароль
    const char Password[] PROGMEM = "\xA8\x61\x70\x6F\xBB\xC4";
    /// АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЭЮЯ01234567890_
    const char password_chars[] PROGMEM = "\x41\xA0\x42\xA1\xE0\x45\xA2\xA3\xA4\xA5\xA6\x4B\xA7\x4D\x48\x4F\xA8\x50\x43\x54\xA9\xAA\x58\xE1\xAB\xAC\xE2\xAD\xAE\xAF\xB0\xB1\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x30\x5F";
    /// Установка пароля
    const char Setting_password[] PROGMEM = "\xA9\x63\xBF\x61\xBD\x6F\xB3\xBA\x61\x20\xBE\x61\x70\x6F\xBB\xC7";
    /// Закрыть доступ?
    const char Close_access_query[] PROGMEM = "\xA4\x61\xBA\x70\xC3\xBF\xC4\x20\xE3\x6F\x63\xBF\x79\xBE\x3F";
    /// Доступ оператор
    const char Access_of_operator_level[] PROGMEM = "\xE0\x6F\x63\xBF\x79\xBE\x20\x6F\xBE\x65\x70\x61\xBF\x6F\x70";
    /// Настройка
    const char Setup[] PROGMEM = "\x48\x61\x63\xBF\x70\x6F\xB9\xBA\x61";
    /// Установка часов
    const char Set_clocks[] PROGMEM = "\xA9\x63\xBF\x61\xBD\x6F\xB3\xBA\x61\x20\xC0\x61\x63\x6F\xB3";
    /// Дата
    const char Date[] PROGMEM = "\xE0\x61\xBF\x61";
    /// Время
    const char Time[] PROGMEM = "\x42\x70\x65\xBC\xC7";
    /// Конфигурация
    const char Configuration[] PROGMEM = "\x4B\x6F\xBD\xE4\xB8\xB4\x79\x70\x61\xE5\xB8\xC7";
    /// Термокомпен. да
    const char Termocompensation_yes[] PROGMEM = "\x54\x65\x70\xBC\x6F\xBA\x6F\xBC\xBE\x65\xBD\x2E\x20\xE3\x61";
    /// Термокомпен. нет
    const char Termocompensation_no[] PROGMEM = "\x54\x65\x70\xBC\x6F\xBA\x6F\xBC\xBE\x65\xBD\x2E\x20\xBD\x65\xBF";
    /// Индикация прив.
    const char Indication_reduced[] PROGMEM = "\xA5\xBD\xE3\xB8\xBA\x61\xE5\xB8\xC7\x20\xBE\x70\xB8\xB3\x2E";
    /// Индикация текущ.
    const char Indication_current[] PROGMEM = "\xA5\xBD\xE3\xB8\xBA\x61\xE5\xB8\xC7\x20\xBF\x65\xBA\x79\xE6\x2E";
    /// Плотность измер.
    const char Dencity_measurement[] PROGMEM = "\xA8\xBB\x6F\xBF\xBD\x6F\x63\xBF\xC4\x20\xB8\xB7\xBC\x65\x70\x2E";
    /// Плотность ввод.
    const char Dencity_constant[] PROGMEM = "\xA8\xBB\x6F\xBF\xBD\x6F\x63\xBF\xC4\x20\xB3\xB3\x6F\xE3\x2E";
    /// Время зап.
    const char Time_of_writing[] PROGMEM = "\x42\x70\x65\xBC\xC7\x20\xB7\x61\xBE\x2E";
    /// сек
    const char sec[] PROGMEM = "\x63\x65\xBA";
    /// мин
    const char min[] PROGMEM = "\xBC\xB8\xBD";
    /// Записано
    const char Writed[] PROGMEM = "\xA4\x61\xBE\xB8\x63\x61\xBD\x6F";
    /// Верхний уровень
    const char Top_level[] PROGMEM = "\x42\x65\x70\x78\xBD\xB8\xB9\x20\x79\x70\x6F\xB3\x65\xBD\xC4";
    /// Реле норм. выкл.
    const char Relay_normal_off[] PROGMEM = "\x50\x65\xBB\x65\x20\xBD\x6F\x70\xBC\x2E\x20\xB3\xC3\xBA\xBB\x2E";
    /// Реле норм. вкл.
    const char Relay_normal_on[] PROGMEM = "\x50\x65\xBB\x65\x20\xBD\x6F\x70\xBC\x2E\x20\xB3\xBA\xBB\x2E";
    /// Канал запмоинает
    const char Chanel_memorize[] PROGMEM = "\x4B\x61\xBD\x61\xBB\x20\xB7\x61\xBE\xBC\x6F\xB8\xBD\x61\x65\xBF";
    /// Канал сквозной
    const char Chanel_through[] PROGMEM = "\x4B\x61\xBD\x61\xBB\x20\x63\xBA\xB3\x6F\xB7\xBD\x6F\xB9";
    /// Звук нет
    const char Sound_off[] PROGMEM = "\xA4\xB3\x79\xBA\x20\xBD\x65\xBF";
    /// Звук 1Гц
    const char Sound_1Gz[] PROGMEM = "\xA4\xB3\x79\xBA\x20\x31\xA1\xE5";
    /// Звук 2Гц
    const char Sound_2Gz[] PROGMEM = "\xA4\xB3\x79\xBA\x20\x32\xA1\xE5";
    /// Уставка
    const char Setting[] PROGMEM = "\xA9\x63\xBF\x61\xB3\xBA\x61";
    /// Нижний уровень
    const char Low_lewel[] PROGMEM = "\x48\xB8\xB6\xBD\xB8\xB9\x20\x79\x70\x6F\xB3\x65\xBD\xC4";
    /// Параметры RS-485
    const char Parameters_of_RS485[] PROGMEM = "\xA8\x61\x70\x61\xBC\x65\xBF\x70\xC3\x20\x52\x53\x2D\x34\x38\x35";
    /// Сетевой адр.
    const char Network_address[] PROGMEM = "\x43\x65\xBF\x65\xB3\x6F\xB9\x20\x61\xE3\x70\x2E";
    /// Бод
    const char Baud[] PROGMEM = "\xA0\x6F\xE3";
    /// Калибровка
    const char Calibration[] PROGMEM = "\x4B\x61\xBB\xB8\xB2\x70\x6F\xB3\xBA\x61";
    /// датчика
    const char Sensor[] PROGMEM = "\xE3\x61\xBF\xC0\xB8\xBA\x61";
    /// АЦП температуры
    const char ADC_of_temperature[] PROGMEM = "\x41\xE1\xA8\x20\xBF\x65\xBC\xBE\x65\x70\x61\xBF\x79\x70\xC3";
    /// мА
    const char mA[] PROGMEM = "\xBC\x41";
    /// АЦП плотности
    const char ADC_of_dencity[] PROGMEM = "\x41\xE1\xA8\x20\xBE\xBB\x6F\xBF\xBD\x6F\x63\xBF\xB8";
    /// Юстировка
    const char Alignment[] PROGMEM = "\xB0\x63\xBF\xB8\x70\x6F\xB3\xBA\x61";
    /// ЦАП вязкости
    const char DAC_of_viscosity[] PROGMEM = "\xE1\x41\xA8\x20\xB3\xC7\xB7\xBA\x6F\x63\xBF\xB8";
    /// Диапазон
    const char Range[] PROGMEM = "\xE0\xB8\x61\xBE\x61\xB7\x6F\xBD";
    /// ВВН-8-011
    const char VVN_8_011[] PROGMEM = "\x42\x42\x48\x2D\x38\x2D\x30\x31\x31";
    /// ВВН-8-021
    const char VVN_8_021[] PROGMEM = "\x42\x42\x48\x2D\x38\x2D\x30\x32\x31";
    /// ВВН-8-031
    const char VVN_8_031[] PROGMEM = "\x42\x42\x48\x2D\x38\x2D\x30\x33\x31";
    /// ВВН-8-041
    const char VVN_8_041[] PROGMEM = "\x42\x42\x48\x2D\x38\x2D\x30\x34\x31";
    /// ВВН-8-051
    const char VVN_8_051[] PROGMEM = "\x42\x42\x48\x2D\x38\x2D\x30\x35\x31";
    /// Uацп
    const char Uadc[] PROGMEM = "\x55\x61\xE5\xBE";
}
