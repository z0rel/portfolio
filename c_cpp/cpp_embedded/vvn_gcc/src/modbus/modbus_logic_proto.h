/*
 * modbus_logic_proto.h
 *
 * Created: 07.04.2022 6:38:12
 *  Author: artem
 */


#ifndef MODBUS_LOGIC_PROTO_H_
#define MODBUS_LOGIC_PROTO_H_

#include <stdint.h>

namespace modbus_rtu {


enum RegistersLogic : uint8_t {
    /// Версия протокола - константа для проверки валидности протокола
    PROTOCOL_VERSION__RO = 0,

    /// Код состояния последней команды записи
    RETURN_CODE_OF_LAST_WRITE__WO = 1,

    /// Запрос на открытие полного доступа в секундах - только запись
    OPEN_ACCESS_QUERY_TIME_SEC__WO = 2,

    /// Команда - Открыть доступ Мастер, Открыть доступ Полный, Открыть доступ Поверитель, Сбросить доступ, Сбросить архив
    COMMAND__WO = 3,

    /// Пароль для запрашиваемого уровня доступа
    /// Формат хранения  пароля _АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ0123456789 - 44 символа.
    /// Кодировка символов пароля - CP1251
    PASSWORD__0_1__WO = 4,
    PASSWORD__2_3__WO = 5,
    PASSWORD__4_5__WO = 6,
    PASSWORD__6_7__WO = 7,

    /// [byte0] = Сетевой адрес: 1-247 + [byte1] = Скорость обмена - UART_Baudrate
    MODBUS__ADDRESS__BAUDRATE__RW = 8,

    /// Год
    DATE__YEAR__RW = 9,
    /// Месяц
    DATE_MONTH__RW = 10,
    /// День месяца
    DATE_DAY_OF_MONTH__RW = 11,
    /// Час
    DATE_HOUR__RW = 12,
    /// Минута
    DATE_MINUTE__RW = 13,
    /// Секунда
    DATE_SECOND__RW = 14,
    /// Миллисекунда
    DATE_MILLISECOND__RW = 15,

    /// Вязкость в кодах АЦП
    ADC_CODE__VIISCOSITY__RO = 16,

    /// Ток по каналу температуры в кодах АЦП
    ADC_CODE__TEMPERATURE__RO = 17,

    /// Ток по каналу плот. в кодах АЦП
    ADC_CODE__DENCITY__RO = 18,

    /// Код ЦАП
    DAC_CODE__RO = 19,

    /// Частота звуковых сигналов
    SOUND_FREQUENCY__RW = 20,

    /// Значение кинематической вязкости срабатывания уставки нижнего уровня * 10 ^ позиция_десятичной_точки
    /// -> округленное до целого
    SETPOINT_LOW__LEVEL_OF_RELAY_ACTIVATE__RW__N0__LSW           = 21,  // младшее слово
    SETPOINT_LOW__LEVEL_OF_RELAY_ACTIVATE__RW__N0__MSW           = 22,  // старшее слово
    SETPOINT_LOW__LEVEL_OF_RELAY_ACTIVATE__RW__N0__DECIMAL_POINT = 23,  // позиция десятичной точки
    SETPOINT_LOW__LEVEL_OF_RELAY_ACTIVATE__RW__N0__DOUBLE__W3    = 24,  // запасное слово для вывода байтов 7-8 формата double

    /// Значение кинематической вязкости срабатывания уставки верхнего уровня * 10 ^ позиция_десятичной_точки
    /// -> округленное до целого
    SETPOINT_HIGH__LEVEL_OF_RELAY_ACTIVATE__RW__N1__LSW           = 25,  // младшее слово
    SETPOINT_HIGH__LEVEL_OF_RELAY_ACTIVATE__RW__N1__MSW           = 26,  // старшее слово
    SETPOINT_HIGH__LEVEL_OF_RELAY_ACTIVATE__RW__N1__DECIMAL_POINT = 27,  // позиция десятичной точки
    SETPOINT_HIGH__LEVEL_OF_RELAY_ACTIVATE__RW__N1__DOUBLE__W3    = 28,  // запасное слово для вывода байтов 7-8 формата double

    /// Значение введённой константы плотности * 10 ^ позиция_десятичной_точки -> округленное до целого
    DENCITY_SETTED__RW__N2__LSW           = 29,  // младшее слово
    DENCITY_SETTED__RW__N2__MSW           = 30,  // старшее слово
    DENCITY_SETTED__RW__N2__DECIMAL_POINT = 31,  // позиция десятичной точки
    DENCITY_SETTED__RW__N2__DOUBLE__W3    = 32,  // запасное слово для вывода байтов 7-8 формата double

    /// Настройка датчика температуры - нижняя температура * 10 ^ позиция_десятичной_точки -> округленное до целого
    CALIBRATION__LOW_TEMPERATURE__RW__N3__LSW           = 33,  // младшее слово
    CALIBRATION__LOW_TEMPERATURE__RW__N3__MSW           = 34,  // старшее слово
    CALIBRATION__LOW_TEMPERATURE__RW__N3__DECIMAL_POINT = 35,  // позиция десятичной точки
    CALIBRATION__LOW_TEMPERATURE__RW__N3__DOUBLE__W3    = 36,  // запасное слово для вывода байтов 7-8 формата double

    /// Настройка датчика температуры - верхняя температура * 10 ^ позиция_десятичной_точки -> округленное до целого
    CALIBRATION__HIGH_TEMPERATURE__RW__N4__LSW           = 37,  // младшее слово
    CALIBRATION__HIGH_TEMPERATURE__RW__N4__MSW           = 38,  // старшее слово
    CALIBRATION__HIGH_TEMPERATURE__RW__N4__DECIMAL_POINT = 39,  // позиция десятичной точки
    CALIBRATION__HIGH_TEMPERATURE__RW__N4__DOUBLE__W3    = 40,  // запасное слово для вывода байтов 7-8 формата double

    /// Настройка датчика плотности - нижняя плотность * 10 ^ позиция_десятичной_точки -> округленное до целого
    CALIBRATION__LOW_DENCITY__RW__N5__LSW           = 41,  // младшее слово
    CALIBRATION__LOW_DENCITY__RW__N5__MSW           = 42,  // старшее слово
    CALIBRATION__LOW_DENCITY__RW__N5__DECIMAL_POINT = 43,  // позиция десятичной точки
    CALIBRATION__LOW_DENCITY__RW__N5__DOUBLE__W3    = 44,  // запасное слово для вывода байтов 7-8 формата double

    /// Настройка датчика плотности - верхняя плотность * 10 ^ позиция_десятичной_точки -> округленное до целого
    CALIBRATION__HIGH_DENCITY__RW__N6__LSW           = 45,  // младшее слово
    CALIBRATION__HIGH_DENCITY__RW__N6__MSW           = 46,  // старшее слово
    CALIBRATION__HIGH_DENCITY__RW__N6__DECIMAL_POINT = 47,  // позиция десятичной точки
    CALIBRATION__HIGH_DENCITY__RW__N6__DOUBLE_W3     = 48,  // запасное слово для вывода байтов 7-8 формата double

    /// Юстировка тока датчика плотности - ток * 10 ^ позиция_десятичной_точки -> округленное до целого
    ADJUST__DENCITY_CURRENT__RW__N7__LSW           = 49,  // младшее слово
    ADJUST__DENCITY_CURRENT__RW__N7__MSW           = 50,  // старшее слово
    ADJUST__DENCITY_CURRENT__RW__N7__DECIMAL_POINT = 51,  // позиция десятичной точки
    ADJUST__DENCITY_CURRENT__RW__N7__DOUBLE__W3    = 52,  // запасное слово для вывода байтов 7-8 формата double

    /// Юстировка тока датчика температуры - ток * множитель тока -> округленное до целого
    ADJUST__TEMPERATURE_CURRENT__RW__N8__LSW           = 53,  // младшее слово
    ADJUST__TEMPERATURE_CURRENT__RW__N8__MSW           = 54,  // старшее слово
    ADJUST__TEMPERATURE_CURRENT__RW__N8__DECIMAL_POINT = 55,  // позиция десятичной точки
    ADJUST__TEMPERATURE_CURRENT__RW__N8__DOUBLE__W3    = 56,  // запасное слово для вывода байтов 7-8 формата double

    /// Юстировка выходного тока датчика вязкости - ток * множитель тока -> округленное до целого
    ADJUST__OUTPUT_VISCOSITY_CURRENT__RW__N9__LSW           = 57,  // младшее слово
    ADJUST__OUTPUT_VISCOSITY_CURRENT__RW__N9__MSW           = 58,  // старшее слово
    ADJUST__OUTPUT_VISCOSITY_CURRENT__RW__N9__DECIMAL_POINT = 59,  // позиция десятичной точки
    ADJUST__OUTPUT_VISCOSITY_CURRENT__RW__N9__DOUBLE__W3    = 60,  // запасное слово для вывода байтов 7-8 формата double

    /// Измеренная кинематическая вязкость * множитель вязкости для типа устройства -> округленное до целого
    MEASURED__VISCOSITY_KINEMATIC__RO__N10__LSW           = 61,  // младшее слово
    MEASURED__VISCOSITY_KINEMATIC__RO__N10__MSW           = 62,  // старшее слово
    MEASURED__VISCOSITY_KINEMATIC__RO__N10__DECIMAL_POINT = 63,  // позиция десятичной точки
    MEASURED__VISCOSITY_KINEMATIC__RO__N10__DOUBLE__W3    = 64,  // запасное слово для вывода байтов 7-8 формата double

    /// Измеренная динамическая вязкость (Вязкость приведённая - в формате double64)
    MEASURED__VISCOSITY_DYNAMIC__RO__N11__LSW           = 65,  // младшее слово
    MEASURED__VISCOSITY_DYNAMIC__RO__N11__MSW           = 66,  // старшее слово
    MEASURED__VISCOSITY_DYNAMIC__RO__N11__DECIMAL_POINT = 67,  // позиция десятичной точки
    MEASURED__VISCOSITY_DYNAMIC__RO__N11__DOUBLE__W3    = 68,  // запасное слово для вывода байтов 7-8 формата double

    /// Измеренная температура
    MEASURED__TEMPERATURE__RO__N12__LSW           = 69,  // младшее слово
    MEASURED__TEMPERATURE__RO__N12__MSW           = 70,  // старшее слово
    MEASURED__TEMPERATURE__RO__N12__DECIMAL_POINT = 71,  // позиция десятичной точки
    MEASURED__TEMPERATURE__RO__N12__DOUBLE__W3    = 72,  // запасное слово для вывода байтов 7-8 формата double

    /// Измеренная плотность
    MEASURED__DENCITY__RO__N13__LSW           = 73,  // младшее слово
    MEASURED__DENCITY__RO__N13__MSW           = 74,  // старшее слово
    MEASURED__DENCITY__RO__N13__DECIMAL_POINT = 75,  // позиция десятичной точки
    MEASURED__DENCITY__RO__N13__DOUBLE__W3    = 76,  // запасное слово для вывода байтов 7-8 формата double

    /// Ток по каналу температуры с юстировкой
    CURRENT__TEMPERATURE_ADJUSTED__RO__N14__LSW           = 77,  // младшее слово
    CURRENT__TEMPERATURE_ADJUSTED__RO__N14__MSW           = 78,  // старшее слово
    CURRENT__TEMPERATURE_ADJUSTED__RO__N14__DECIMAL_POINT = 79,  // позиция десятичной точки
    CURRENT__TEMPERATURE_ADJUSTED__RO__N14__DOUBLE__W3    = 80,  // запасное слово для вывода байтов 7-8 формата double

    /// Ток по каналу плотности с юстировкой
    CURRENT__DENCITY_ADJUSTED__RO__N15__LSW           = 81,  // младшее слово
    CURRENT__DENCITY_ADJUSTED__RO__N15__MSW           = 82,  // старшее слово
    CURRENT__DENCITY_ADJUSTED__RO__N15__DECIMAL_POINT = 83,  // позиция десятичной точки
    CURRENT__DENCITY_ADJUSTED__RO__N15__DOUBLE__W3    = 84,  // запасное слово для вывода байтов 7-8 формата double

    /// Значение напряжения на выходе операционного усилителя датчика вязкости
    VOLTAGE__VISCOSITY_INPUT_CANNEL__RO__N16__LSW           = 85,  // младшее слово
    VOLTAGE__VISCOSITY_INPUT_CANNEL__RO__N16__MSW           = 86,  // старшее слово
    VOLTAGE__VISCOSITY_INPUT_CANNEL__RO__N16__DECIMAL_POINT = 87,  // позиция десятичной точки
    VOLTAGE__VISCOSITY_INPUT_CANNEL__RO__N16__DOUBLE__W3    = 88,  // запасное слово для вывода байтов 7-8 формата double

    _TABLE_SIZE = 89,
    _MAX_MESSAGE_SIZE = _TABLE_SIZE * 2 + 5,
};


enum RegistersLogicCoils : uint8_t {
    /// Битовая маска настройки уставок
    /// [0] Уставка нижнего уровня включена или выключена
    SETPOINT_LOWLEVEL_ENABLE = 0,
    /// [1] Звуковой сигнал уставки нижнего уровня включен или выключен
    SETPOINT_LOWLEVEL_SOUND_ENABLE = 1,
    /// [2] Тип обработки уставки нижнего уровня: 0 = сквозной или 1 = запоминающий
    SETPOINT_LOWLEVEL_TYPE = 2,

    /// [3] Уставка верхнего уровня включена или выключена
    SETPOINT_HIGHLEVEL_ENABLE = 3,
    /// [4] Звуковой сигнал уставки верхнего уровня включен или выключен
    SETPOINT_HIGHEVEL_SOUND_ENABLE = 4,
    /// [5] Тип обработки уставки верхнего уровня: сквозной или запоминающий
    SETPOINT_HIGHEVEL_TYPE = 5,

    /// Числовые форматы всех 17 численных выводимых значений - 0 = NUMERIC|1 = FLOAT32|2 = FLOAT64
    FORMAT__SETPOINT_LOW__LEVEL_OF_RELAY_ACTIVATE__0 = 6,
    FORMAT__SETPOINT_LOW__LEVEL_OF_RELAY_ACTIVATE__1 = 7,

    FORMAT__SETPOINT_HIGH__LEVEL_OF_RELAY_ACTIVATE__0 = 8, 
    FORMAT__SETPOINT_HIGH__LEVEL_OF_RELAY_ACTIVATE__1 = 9,

    FORMAT__DENCITY_SETTED__0 = 10,
    FORMAT__DENCITY_SETTED__1 = 11,
	
    FORMAT__CALIBRATION__LOW_TEMPERATURE__0 = 12,
    FORMAT__CALIBRATION__LOW_TEMPERATURE__1 = 13,

    FORMAT__CALIBRATION__HIGH_TEMPERATURE__0 = 14,
    FORMAT__CALIBRATION__HIGH_TEMPERATURE__1 = 15,

    FORMAT__CALIBRATION__LOW_DENCITY__0 = 16,
    FORMAT__CALIBRATION__LOW_DENCITY__1 = 17,

    FORMAT__CALIBRATION__HIGH_DENCITY__0 = 18,
    FORMAT__CALIBRATION__HIGH_DENCITY__1 = 19,

    FORMAT__ADJUST__DENCITY_CURRENT__0 = 20,
    FORMAT__ADJUST__DENCITY_CURRENT__1 = 21,

    FORMAT__ADJUST__TEMPERATURE_CURRENT__0 = 22,
    FORMAT__ADJUST__TEMPERATURE_CURRENT__1 = 23,

    FORMAT__ADJUST__OUTPUT_VISCOSITY_CURRENT__0 = 24,
    FORMAT__ADJUST__OUTPUT_VISCOSITY_CURRENT__1 = 25,

    FORMAT__MEASURED__VISCOSITY_KINEMATIC__0 = 26,
    FORMAT__MEASURED__VISCOSITY_KINEMATIC__1 = 27,

    FORMAT__MEASURED__VISCOSITY_DYNAMIC__0 = 28,
    FORMAT__MEASURED__VISCOSITY_DYNAMIC__1 = 29,

    FORMAT__MEASURED__TEMPERATURE__0 = 30,
    FORMAT__MEASURED__TEMPERATURE__1 = 31,

    FORMAT__MEASURED__DENCITY__0 = 32,
    FORMAT__MEASURED__DENCITY__1 = 33,

    FORMAT__CURRENT__TEMPERATURE_ADJUSTED__0 = 34,
    FORMAT__CURRENT__TEMPERATURE_ADJUSTED__1 = 35,

    FORMAT__CURRENT__DENCITY_ADJUSTED__0 = 36,
    FORMAT__CURRENT__DENCITY_ADJUSTED__1 = 37,

    FORMAT__VOLTAGE__VISCOSITY_INPUT_CANNEL__0 = 38,
    FORMAT__VOLTAGE__VISCOSITY_INPUT_CANNEL__1 = 39,
    
	_COIL_MAX = 40,
	_COIL_MAX_TABLE_SIZE = 1 + 40 / 16
};


}  // namespace modbus_rtu


#endif /* MODBUS_LOGIC_PROTO_H_ */
