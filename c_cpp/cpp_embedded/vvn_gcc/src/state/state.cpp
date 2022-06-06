/*
 * state.cpp
 *
 * Created: 16.03.2022 11:09:30
 *  Author: artem
 */

#include "../../arduino/Arduino.h"
#include "../../coroutine/Coroutine.h"
#include "../../coroutines/display_context.h"
#include "../config_vvn8/config_vvn8.h"
#include "../init_extram.h"
#include "../lcd/lcd_literals.h"
#include "../lcd/lcd_literals_objs.h"
#include "../rtc.h"
#include "hmi_state.h"
#include "state_utils.h"


namespace state {


bool waitDisplayVaild() { return coroutines::displayContext.isValid(); }
bool waitDisplayVaildFull() { return coroutines::displayContext.isValidFull(); }

void set_initial_state()
{
    state::state_output_current.hmi_menu_item = HMI_Menu::TEST;
    memcpy_P(state_output_current.row0, LCD_constants::Test_memory, LENGTH_Test_memory);
}


void set_state_main_with_time()
{
    // char g = pgm_read_byte(LCD_constants::g);
    DateTime dt;
    dt.load();
    // day(1).month(2).century(3) year(4) hour(5):minute(6):second(7)
    snprintf(state_output_current.row0, TEXT_BUFFER_LENGTH_SNPRINTF, "%2i.%02i.%02i%02i %02i:%02i", dt.tm_mday, dt.tm_mon, dt.tm_century, dt.tm_year, dt.tm_hour, dt.tm_min);
    snprintf(state_output_current.row1, TEXT_BUFFER_LENGTH_SNPRINTF, "      ");
    memcpy_P(state_output_current.row1 + 6, LCD_constants::Pa_s_kg_m3, LENGTH_Pa_s_kg_m3);
}


void set_logo_state()
{
    step_state_logo(3);
    set_row_text(state_output_current.row1, LCD_constants::Logo_Automatics, LENGTH_Logo_Automatics);
    set_menu_state(HMI_Menu::LOGO);
}


void step_state_logo(uint8_t shift)
{
    switch (config::config_vvn8.vvn_type) {
    case config::VVN_TYPE::VVN8_011:
        set_row_text_shift(state_output_current.row0, LCD_constants::VVN_8_011, LENGTH_VVN_8_011, shift);
        break;
    case config::VVN_TYPE::VVN8_021:
        set_row_text_shift(state_output_current.row0, LCD_constants::VVN_8_021, LENGTH_VVN_8_021, shift);
        break;
    case config::VVN_TYPE::VVN8_031:
        set_row_text_shift(state_output_current.row0, LCD_constants::VVN_8_031, LENGTH_VVN_8_031, shift);
        break;
    case config::VVN_TYPE::VVN8_041:
        set_row_text_shift(state_output_current.row0, LCD_constants::VVN_8_041, LENGTH_VVN_8_041, shift);
        break;
    case config::VVN_TYPE::VVN8_051:
        set_row_text_shift(state_output_current.row0, LCD_constants::VVN_8_051, LENGTH_VVN_8_051, shift);
        break;
    default:
        break;
    }
}


COROUTINE(state_next, void)
{
    COROUTINE_LOOP()
    {
        switch (state::state_output_current.hmi_menu_item) {
            // Тест ОЗУ -> OK
        case HMI_Menu::TEST: {
            WAIT_INVALIDATE_DISPLAY_FULL();
            if (extram_check()) {
                COROUTINE_DELAY_MAIN(1); // 500);
                set_logo_state();
                WAIT_INVALIDATE_DISPLAY_FULL();
            }
            else {
                COROUTINE_DELAY_MAIN(1); //500);
                set_row_text(state_output_current.row1, LCD_constants::Test_memory_error, LENGTH_Test_memory_error);
                set_menu_state(HMI_Menu::TEST_MEMORY_ERROR);
                WAIT_INVALIDATE_DISPLAY_FULL();
            }
        } break;

        case HMI_Menu::TEST_MEMORY_ERROR: {
            COROUTINE_DELAY_MAIN(1); // 1500);
        } break;

            // Логотип автоматики
            // "   ВВН-8    "  | ОАО "Автоматика"
        case HMI_Menu::LOGO: {
            COROUTINE_DELAY_MAIN(1);// 1500);
            set_state_main_with_time();
            set_menu_state(HMI_Menu::MAIN_WITH_TIME);
            WAIT_INVALIDATE_DISPLAY_FULL();
        } break;

            // Главный экран (Бегущая строка времени) | 0.0227 Па * с * кг / м3
        case HMI_Menu::MAIN_WITH_TIME: {
        } break;

            // Главный экран -> F Функции
            // Главный экран -> F Функции -> F Записей нет
            // Главный экран -> F Функции -> F Записей нет -> Вязкость текущая

            // Главный экран -> F Функции -> Архив: 0019 13/03 07:07 | 0.0000Pa*s*kg/m3
        case HMI_Menu::MAIN_ARCHIVE: {
        }

            // Экран "Состояние"
            // (1) Вязкость текущая | 0.0227 Па * с * кг / м3
        case HMI_Menu::MAIN_STATE_VISOCITY_KINEMATIC: {

        } break;
            // DOWN Вязкость привед | 0.0227 Па * с
        case HMI_Menu::MAIN_STATE_VISOCITY_DYNAMIC: {
        }

        break;
            // DOWN Температура | t=46.31 °C
        case HMI_Menu::MAIN_STATE_TEMPERATURE: {
        } break;
            // DOWN Плотность | p=1.000 кг/м3
        case HMI_Menu::MAIN_STATE_DENCITY: {
        } break;

        case HMI_Menu::MAIN_STATE_VISOCITY_KINEMATIC_ADC_CODE: // Сервисный экран: доступный в режиме поверитель - отображающий текущий код АЦП
        {
        } break;

        // Функции F1 -> Функции | Сброс
        case HMI_Menu::MENU_F1__RESET: {
        } break;
            // Функции F1 -> Функции | Очистка архива
        case HMI_Menu::MENU_F1__CLEAR_ARCHIVE: {
        } break;
            // Функции F1 -> Функции | Квитирование ???
        case HMI_Menu::MENU_F1__CVITIR: {
        } break;
            // Функции F -> F Доступ | Открытие доступа
        case HMI_Menu::MENU_F2__ACCESS_OPEN: {
        } break;
            // Функции F -> F Доступ | Закрыть доступ?
        case HMI_Menu::MENU_F2__ACCESS_CLOSE: {
        } break;
            // Функции F -> F ENT Доступ | Закрыть доступ? -> Доступ оператор
        case HMI_Menu::MENU_F2__ACCESS_CLOSE_ENT: {
        } break;
        case HMI_Menu::MENU_F2__OPEN_ACCESS_ENTER_PASSWORD: {
        } break;

            // Функции F -> F Доступ | Установка пароля
        case HMI_Menu::MENU_F2__ACCESS_SET_PASSWORD: {
        } break;
            // Функции F -> F -> F Доступ | Открытие доступа -> Доступ поверит. | Пароль _ _ _ _ _ _ _ _

            // TODO: уровни доступа Полный и Мастер определяются только паролем.
            // Полный - пароль Б
            // Мастер - пароль В
            // Поверит - пустой пароль
            // Формат пароля _АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ0123456789

            // После открытия доступа
        case HMI_Menu::MENU_F3__SETUP_CLOCK: {
        } break;
            // Функции F -> F -> F Настройка | Установка часов
        case HMI_Menu::MENU_F3__SETUP_CLOCK_ENT: {
        } break;
            // Функции F -> F -> F Настройка | Установка часов ENT -> Дата 13.03.22 | Время 04:00:46
            // Функции F -> F -> F Настройка | Установка часов ENT -> DOWN Дата [12].03.22 | Время 04:00:46
            // TIRE - + единица
            // [] - мигание
            // F -> Дата 12.[03].22 | Время 04:00:46 -> F Дата 12.03.[22] | Время 04:00:46 -> F Дата 12.03.22 | Время [04]:00:46
            // Ent - выход из мигания, Ent - сохранение, Esc - выход в предыдущее меню

            // Функции F -> F -> F Настройка | Параметры RS-485
        case HMI_Menu::MENU_F3__SETUP_RS485: {
        } break;
            // Функции F -> F -> F Настройка | Параметры RS-485 ENT -> Сетевой адр. 000 | 2400 Бод
        case HMI_Menu::MENU_F3__SETUP_RS485_ENT: {
        } break;
            // (вниз - меняет сетевой адрес на 255)
        case HMI_Menu::MENU_F3__SETUP_RS485_ENT__SET_BAUDRATE: {
        } break;
            // (F - меняет бодрейт на 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 76800, 115200)
        case HMI_Menu::MENU_F3__SETUP_RS485_ENT__SET_MODUBS_ADDRESS: {
        } break;
            // Ent - записывает

            // Функции F -> F -> F Настройка | Нижний уровень
        case HMI_Menu::MENU_F3__SETUP_LOW_RELAY: {
        } break;
            // Функции F -> F -> F Настройка | Нижний уровень ENT -> Нижний уровень | Реле норм. выкл
        case HMI_Menu::MENU_F3__SETUP_LOW_RELAY__ENT__RELAY_STATE: {
        } break;
            // Функции F -> F -> F Настройка | Нижний уровень ENT Down -> Нижний уровень | Реле норм. вкл.
            // Enter
            // Функции F -> F -> F Настройка | Нижний уровень ENT Down -> Записано | Реле норм. вкл.
        case HMI_Menu::MENU_F3__SETUP_LOW_RELAY__ENT__WRITED: {
        } break;
            //   1 sec
            // Функции F -> F -> F Настройка | Нижний уровень ENT Down -> Нижний уровень | Реле норм. вкл.
            // Функции F -> F -> F Настройка | Нижний уровень ENT F -> Нижний уровень | Канал запомниает
        case HMI_Menu::MENU_F3__SETUP_LOW_RELAY__ENT__CHANEL_STATE: {
        } break;
            // Функции F -> F -> F Настройка | Нижний уровень ENT F Down -> Нижний уровень | Канал сквозной

            // Функции F -> F -> F Настройка | Нижний уровень ENT F -> Нижний уровень | Звук нет
        case HMI_Menu::MENU_F3__SETUP_LOW_RELAY__ENT__SOUND: {
        } break;
            // Функции F -> F -> F Настройка | Нижний уровень ENT F DOWN -> Нижний уровень | 1 Гц
            // Функции F -> F -> F Настройка | Нижний уровень ENT F DOWN -> Нижний уровень | 2 Гц

            // Функции F -> F -> F Настройка | Нижний уровень ENT F -> Нижний уровень | Уставка 0.0000
        case HMI_Menu::MENU_F3__SETUP_LOW_RELAY__ENT__SETTING_VALUE: {
        } break;
            // Функции F -> F -> F Настройка | Нижний уровень ENT F DOWN -> Нижний уровень | Уставка 9.0000
            // ...
            // Функции F -> F -> F Настройка | Нижний уровень ENT F DOWN -> Нижний уровень | Уставка 1.0000
            // Функции F -> F -> F Настройка | Нижний уровень ENT F TIRE -> Нижний уровень | Уставка 2.0000
            // ...
            // Функции F -> F -> F Настройка | Нижний уровень ENT F TIRE -> Нижний уровень | Уставка 9.0000
            // Функции F -> F -> F Настройка | Нижний уровень ENT F DOWN | TIRE -> Нижний уровень | Уставка 9[.]0000   []=>[.0123456789]
            // Функции F -> F -> F Настройка | Нижний уровень ENT F DOWN | TIRE ENTER -> Нижний уровень | Уставка 9.0000  -< Символы не мигают
            // Функции F -> F -> F Настройка | Нижний уровень ENT F DOWN | TIRE ENTER ENTER -> Записано | Уставка 9.0000  -< Символы не мигают
            //   1 sec
            // Функции F -> F -> F Настройка | Нижний уровень ENT F DOWN | TIRE ENTER ENTER -> Нижний уровень | Уставка 9.0000  -< Символы не мигают
            // Функции F -> F -> F Настройка | Нижний уровень ENT Down -> Нижний уровень | Реле норм. вкл.

            // Функции F -> F -> F Настройка | Верхний уровень
        case HMI_Menu::MENU_F3__kSETUP_HIGH_RELAY: {
        } break;
        case HMI_Menu::MENU_F3__kSETUP_HIGH_RELAY__ENT__RELAY_STATE: {
        } break;
        case HMI_Menu::MENU_F3__kSETUP_HIGH_RELAY__ENT__WRITED: {
        } break;
        case HMI_Menu::MENU_F3__kSETUP_HIGH_RELAY__ENT__CHANEL_STATE: {
        } break;
        case HMI_Menu::MENU_F3__kSETUP_HIGH_RELAY__ENT__SOUND: {
        } break;
        case HMI_Menu::MENU_F3__kSETUP_HIGH_RELAY__ENT__SETTING_VALUE: {
        } break;
            // ...

            // Функции F -> F -> F Настройка | Конфигурация
        case HMI_Menu::MENU_F3__CONFIG: {
        } break;
            // Функции F -> F -> F Настройка | Конфигурация ENT -> Конфигурация | Термокомпен. да
        case HMI_Menu::MENU_F3__CONFIG__TERMOCOMPENSATION: {
        } break;
            // Функции F -> F -> F Настройка | Конфигурация ENT DOWN -> Конфигурация | Термокомпен. нет
            // или если доступ не открыт
            // Функции F -> F -> F Настройка | Конфигурация ENT DOWN -> нет доступа | Термокомпен. нет

        case HMI_Menu::MENU_F3__CONFIG__VISCOSITY_INDICATION_TYPE: {
        } break;
            // Функции F -> F -> F Настройка | Конфигурация ENT F -> Конфигурация | Индикация прив.
            // Функции F -> F -> F Настройка | Конфигурация ENT F DOWN -> Конфигурация | Индикация текущ.

            // Функции F -> F -> F Настройка | Конфигурация ENT F -> Конфигурация | Плотность измер. // dflt
        case HMI_Menu::MENU_F3__CONFIG__DENCITY_GETTER_TYPE: {
        } break;
            // Функции F -> F -> F Настройка | Конфигурация ENT F -> Конфигурация | Плотность ввод.
            // TODO: добавить ввод плотности введённой
        case HMI_Menu::MENU_F3__CONFIG__DENCITY_CONST_INPUT: {
        } break;

            // Функции F -> F -> F Настройка | Конфигурация ENT F -> Конфигурация | Время зап. 5сек
        case HMI_Menu::MENU_F3__CONFIG__INTERVAL_OF_LOGGING: {
        } break;
            // Функции F -> F -> F Настройка | Конфигурация ENT F TIRE -> Конфигурация | Время зап. 10 сек
            // 5сек -> 10сек -> 20сек -> 30сек -> 40сек -> 50сек -> 60сек -> 1мин -> 2мин -> 3мин -> 4мин -> 5мин -> 6мин -> 7мин -> 8мин -> 9мин -> 10мин
            //
            // Функции F -> F -> F -> F Калибровка | АЦП температуры
        case HMI_Menu::MENU_F4__CALIBRATION_ADC_TEMPERATURE: {
        } break;
            // Функции F -> F -> F -> F ENT Калибровка | АЦП температуры -> 4ma -0.900°C | 20ma 200.00°C
        case HMI_Menu::MENU_F4__CALIBRATION_ADC_TEMPERATURE_ENT: {
        } break;
        case HMI_Menu::MENU_F4__CALIBRATION_ADC_TEMPERATURE_ENT_INPUT_4mA: {
        } break;
            // Функции F -> F -> F -> F ENT F Калибровка | АЦП температуры -> 4ma [-]0.900°C | 20ma 200.00°C
        case HMI_Menu::MENU_F4__CALIBRATION_ADC_TEMPERATURE_ENT_INPUT_20mA: {
        } break;
            // Функции F -> F -> F -> F ENT F ENT Калибровка | АЦП температуры -> Записано | 20ma 200.00°C

            // Функции F -> F -> F -> F Калибровка | АЦП плотности
        case HMI_Menu::MENU_F4__CALIBRATION_ADC_DENCITY: {
        } break;
        case HMI_Menu::MENU_F4__CALIBRATION_ADC_DENCITY_ENT: {
        } break;
        case HMI_Menu::MENU_F4__CALIBRATION_ADC_DENCITY_ENT__INPUT_4mA: {
        } break;
            // Функции F -> F -> F -> F ENT Калибровка | АЦП плотности -> 4mA 1.000кг/м3 | 20mA 93.00 кг/м3  (3 - индексом)
        case HMI_Menu::MENU_F4__CALIBRATION_ADC_DENCITY_ENT__INPUT_20mA: {
        } break;

        case HMI_Menu::MENU_F5__ALIGNMENT__ADC_TEMPERATURE: {
        } break;
            // Функции F -> F -> F -> F -> F Юстировка | АЦП температуры
        case HMI_Menu::MENU_F5__ALIGNMENT__ADC_TEMPERATURE__ENT: {
        } break;
        case HMI_Menu::MENU_F5__ALIGNMENT__ADC_TEMPERATURE__ENT__INPUT_CURRENT: {
        } break;
            // Функции F -> F -> F -> F -> F ENT Юстировка | АЦП температуры -> Iвх=18.94 mA | Iизм=18.94 mA
            // Функции F -> F -> F -> F -> F ENT -> F Юстировка | АЦП температуры -> Iвх=18.94 mA | Iизм=1[8].94 mA

        case HMI_Menu::MENU_F5__ALIGNMENT__DAC_VISCOSITY: {
        } break;
        case HMI_Menu::MENU_F5__ALIGNMENT__DAC_VISCOSITY__ENT: {
        } break;
        case HMI_Menu::MENU_F5__ALIGNMENT__DAC_VISCOSITY__ENT__INPUT_CURRENT: {
        } break;
            // Функции F -> F -> F -> F -> F Юстировка | ЦАП вязкости
            // Функции F -> F -> F -> F -> F ENT Юстировка | ЦАП вязкости -> Iвых=10.10 mA | Iизм=10.00 mA
            // Функции F -> F -> F -> F -> F ENT -> F Юстировка | ЦАП вязкости -> Iвых=10.10 mA | Iизм=[1]0.00 mA

        case HMI_Menu::MENU_F5__ALIGNMENT__ADC_DENCITY: {
        } break;
        case HMI_Menu::MENU_F5__ALIGNMENT__ADC_DENCITY__ENT: {
        } break;
        case HMI_Menu::MENU_F5__ALIGNMENT__ADC_DENCITY__ENT__INPUT_CURRENT: {
        } break;
            // Функции F -> F -> F -> F -> F Юстировка | АЦП плотности
            // Функции F -> F -> F -> F -> F ENT Юстировка | АЦП плотности -> Iвх=18.94 mA | Iизм=18.94 mA
            // Функции F -> F -> F -> F -> F ENT -> F Юстировка | АЦП плотности -> Iвх=18.94 mA | Iизм=1[8].94 mA

        case HMI_Menu::MENU_F6__RANGE_VVN8: {
        } break;
            // Функции F -> F -> F -> F -> F -> F Диапазон | ВВН-8-011
            // Функции F -> F -> F -> F -> F -> F DOWN Диапазон | ВВН-8-051
            // Функции F -> F -> F -> F -> F -> F DOWN Диапазон | ВВН-8-041
            // Функции F -> F -> F -> F -> F -> F DOWN Диапазон | ВВН-8-031
            // Функции F -> F -> F -> F -> F -> F DOWN Диапазон | ВВН-8-021
            // Функции F -> F -> F -> F -> F -> F ENT Записано | ВВН-8-021

        case HMI_Menu::MENU_F7__SENSOR_CALIBRATION: {
        } break;
            // Функции F -> F -> F -> F -> F -> F -> Калибровка датчика
        case HMI_Menu::MENU_F7__SENSOR_CALIBRATION__ENT__H1: {
        } break;
        case HMI_Menu::MENU_F7__SENSOR_CALIBRATION__ENT__H2: {
        } break;
        case HMI_Menu::MENU_F7__SENSOR_CALIBRATION__ENT__H3: {
        } break;
        case HMI_Menu::MENU_F7__SENSOR_CALIBRATION__ENT__H4: {
        } break;
        case HMI_Menu::MENU_F7__SENSOR_CALIBRATION__ENT__H5: {
        } break;
        case HMI_Menu::MENU_F7__SENSOR_CALIBRATION__ENT__H6: {
        } break;
        case HMI_Menu::MENU_F7__SENSOR_CALIBRATION__ENT__SAVE_CONFIRMATION: {
        } break;
        case HMI_Menu::MENU_F7__SENSOR_CALIBRATION__ENT__SAVED: {
        } break;
            // Функции F -> F -> F -> F -> F -> F -> ENT Калибровка датчика -> Uацп-1.972-0.650 |  h[1]-19.700
            // Функции F -> F -> F -> F -> F -> F -> ENT F TIRE Калибровка датчика -> Uацп-1.972-0.950 |  h[[2]]-40.890
            // ...
            // Функции F -> F -> F -> F -> F -> F -> ENT F TIRE Калибровка датчика -> Uацп-1.972-2.2533 | h[[6]]-191.22
            // Функции F -> F -> F -> F -> F -> F -> ENT F TIRE Калибровка датчика -> Рассчитать | коэффициенты?
            // Функции F -> F -> F -> F -> F -> F -> ENT F TIRE Калибровка датчика -> коефф. полинома | k[0]=0.0000 k[1]=1.0000000000 k[2]=0.0000000000 k[3]=0.0000000000 k[4]=0.0000000000
            // Функции F -> F -> F -> F -> F -> F -> ENT -> ENT Калибровка датчика -> Записать | коэффициенты ?
            // Функции F -> F -> F -> F -> F -> F -> ENT -> ENT ENT Калибровка датчика -> Записано | h[1]-[1]9.700


            // Функции F -> F -> F -> F -> F -> F -> ENT -> F Калибровка датчика -> Uацп-1.972-0.650 |  h[[1]]-19.700
            // Функции F -> F -> F -> F -> F -> F -> ENT -> F -> F Калибровка датчика -> Uацп-1.972-0.650 |  h[1]-[1]9.700


            // Функции F -> F -> F -> F -> F -> F -> F Сброс


            // DEFAULT
            // Uацп-[1]-[2]:
            //  [1] – отображает текущее значение АЦП
            //  [2] – отображает ?????
            //  [3] – Значение вязкости Па * с * кг / м3 в заданной точке ?
            // Uацп-1.972-1.972 |  h[1]-19.700 было    Uацп-1.972-0.650
            // Uацп-1.972-0.950 |  h[2]-40.890
            // Uацп-1.972-1.392 |  h[3]-80.720
            // Uацп-1.972-1.725 |  h[4]-118.75
            // Uацп-1.972-2.003 |  h[5]-159.39
            // Uацп-1.972-2.253 |  h[6]-191.22


            // Диапазон ВВН-8-041
            // Uацп-1.972-0.650 |  h[1]-19.700
            // h[1] h[2] h[3] h[4] h[5] h[6]
            // Значения вязкостей градуировочных жидко-стей, Па?с?кг/м3 ->
            // Значения напряжений U1...U6  при вязкостях h[1] ... h[6], В
        }
        COROUTINE_YIELD();
    }
}


} // namespace state
