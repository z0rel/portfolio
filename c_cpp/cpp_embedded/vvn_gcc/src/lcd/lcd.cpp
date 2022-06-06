/*
 * lcd.cpp
 *
 * Created: 08.03.2022 15:30:03
 *  Author: artem
 */

#include <stdint.h>

#include <util/delay.h>

#include "../../arduino/Arduino.h"
#include "../../arduino/pins_arduino.h"

#include "../../coroutine/Coroutine.h"
#include "../../coroutine/ctx.h"

#include "../vvn8_pins.h"

#include "lcd_api_context.h"
#include "lcd_commands.h"


namespace LCD_driver {


/// Порт данных LCD
#define PORT_LCD_DATA P_LCD_D7_PORT


static const uint8_t ch_index_3[8] PROGMEM = {
  B11000, //     XX
  B00100, //       X
  B01000, //      X
  B00100, //       X
  B11000, //     XX
  B00000, //
  B00000, //
};

static const uint8_t ch_upgradus[8] PROGMEM = {
  B00110, //       XX
  B01001, //      X  X
  B01001, //      X  X
  B00110, //       XX
  B00000, //
  B00000, //
  B00000, //
};

static const uint8_t ch_muldot[8] PROGMEM = {
  B00000, //
  B00000, //
  B00000, //
  B00110, //      XX
  B00110, //      XX
  B00000, //
  B00000, //
};


/// Генерация верхнего полубайта для записи в LED
static inline uint8_t led_upper_nibble(uint8_t port_data, uint8_t value)
{
    return (((value & _BV(4)) >> (4 - 3)) //  [4]=P_DAC_SCK__LCD_D4  -> PIN_PF3=[3]
            | ((value & _BV(5)) >> (5 - 2)) //  [5]=P_DAC_SDI__LCD_D5  -> PIN_PF2=[2]
            | ((value & _BV(6)) >> (6 - 1)) //  [6]=P_DAC_LDAC__LCD_D6 -> PIN_PF1=[1]
            | ((value & _BV(7)) >> (7 - 0)) //  [7]=P_LCD_D7           -> PIN_PF0=[0]
            ) |
           (port_data & (_BV(P_VVN_ADC7_PPIN) | _BV(P_T_ADC6_PPIN) | _BV(P_P_ADC5_PPIN)));
}


/// Генерация нижнего полубайта для записи в LED
static inline uint8_t led_lower_nibble(uint8_t port_data, uint8_t value)
{
    return (((value & _BV(0)) << (3 - 0)) //  [0]=P_DAC_SCK__LCD_D4  -> PIN_PF3=[3]
            | ((value & _BV(1)) << (2 - 1)) //  [1]=P_DAC_SDI__LCD_D5  -> PIN_PF2=[2]
            | ((value & _BV(2)) >> (2 - 1)) //  [2]=P_DAC_LDAC__LCD_D6 -> PIN_PF1=[1]
            | ((value & _BV(3)) >> (3 - 0)) //  [3]=P_LCD_D7           -> PIN_PF0=[0]
            ) |
           (port_data & (_BV(P_VVN_ADC7_PPIN) | _BV(P_T_ADC6_PPIN) | _BV(P_P_ADC5_PPIN)));
}


void lcd_command_sync(uint8_t command)
{
    // отправка верхнего полубайта
    PORT_LCD_DATA = led_upper_nibble(PORT_LCD_DATA, command);

    P_LCD_RS_PORT &= ~_BV(P_LCD_RS_PPIN); // LCD_RS = 0 : Command Register
    P_LCD_EN_PORT |= _BV(P_LCD_EN_PPIN); // LCD_E = 1 : Enable pulse

    _delay_us(1);
    P_LCD_EN_PORT &= ~_BV(P_LCD_EN_PPIN); // LDC_E = 0

    _delay_us(200);

    // отправка нижнего полубайта
    PORT_LCD_DATA = led_lower_nibble(PORT_LCD_DATA, command);

    P_LCD_EN_PORT |= _BV(P_LCD_EN_PPIN); // LDC_E = 1

    _delay_us(1);

    P_LCD_EN_PORT &= ~_BV(P_LCD_EN_PPIN); // LDC_E = 0

    _delay_ms(2);
}


void lcd_char_sync(unsigned char data)
{
    // отправка верхнего полубайта
    PORT_LCD_DATA = led_upper_nibble(PORT_LCD_DATA, data);
    P_LCD_RS_PORT |= _BV(P_LCD_RS_PPIN); // LCD_RS = 1  : Data register

    P_LCD_EN_PORT |= _BV(P_LCD_EN_PPIN); // LCD_E = 1 : LCD input Enable
    _delay_us(1);
    P_LCD_EN_PORT &= ~_BV(P_LCD_EN_PPIN); // LCD_E = 0 : LCD input disable

    _delay_us(200);

    // отправка нижнего полубайта
    PORT_LCD_DATA = led_lower_nibble(PORT_LCD_DATA, data);

    P_LCD_EN_PORT |= _BV(P_LCD_EN_PPIN); // LCD_E = 1 : LCD input Enable
    _delay_us(1);
    P_LCD_EN_PORT &= ~_BV(P_LCD_EN_PPIN); // LCD_E = 0 : LCD input Enable

    _delay_ms(2);
}


void lcd_char_sync(unsigned char data);


void lcd_create_char(uint8_t location, const uint8_t charmap[] PROGMEM)
{
    location &= 0x7; // we only have 8 locations 0-7

    lcd_command_sync(LCD_SETCGRAMADDR | (location << 3));
    for (int i = 0; i < 8; i++) {
        unsigned char c = pgm_read_byte(charmap + i);
        lcd_char_sync(c);
    }
}


// Отправка команды в LCD
COROUTINE(lcd_command, uint8_t)
{
    COROUTINE_LOOP()
    {
        // Serial.print("LCD Command ");
        // Serial.println(static_cast<uint16_t>(*CTX));

        // отправка верхнего полубайта
        PORT_LCD_DATA = led_upper_nibble(PORT_LCD_DATA, *CTX);

        P_LCD_RS_PORT &= ~_BV(P_LCD_RS_PPIN); // LCD_RS = 0 : Command Register
        P_LCD_EN_PORT |= _BV(P_LCD_EN_PPIN); // LCD_E = 1 : Enable pulse

        _delay_us(1);
        P_LCD_EN_PORT &= ~_BV(P_LCD_EN_PPIN); // LDC_E = 0

        COROUTINE_DELAY_MICROS(200);

        // отправка нижнего полубайта
        PORT_LCD_DATA = led_lower_nibble(PORT_LCD_DATA, *CTX);

        P_LCD_EN_PORT |= _BV(P_LCD_EN_PPIN); // LDC_E = 1

        _delay_us(1);

        P_LCD_EN_PORT &= ~_BV(P_LCD_EN_PPIN); // LDC_E = 0

        COROUTINE_DELAY(2);

        COROUTINE_END_CALL();
    }
}


// Отправка символ в LCD
COROUTINE(lcd_char, uint8_t)
{
    COROUTINE_LOOP()
    {
        // Serial.print("LCD Char ");
        // Serial.println(static_cast<char>(*CTX));

        // отправка верхнего полубайта
        PORT_LCD_DATA = led_upper_nibble(PORT_LCD_DATA, *CTX);
        P_LCD_RS_PORT |= _BV(P_LCD_RS_PPIN); // LCD_RS = 1  : Data register

        P_LCD_EN_PORT |= _BV(P_LCD_EN_PPIN); // LCD_E = 1 : LCD input Enable
        _delay_us(1);
        P_LCD_EN_PORT &= ~_BV(P_LCD_EN_PPIN); // LCD_E = 0 : LCD input disable

        COROUTINE_DELAY_MICROS(200);

        // отправка нижнего полубайта
        PORT_LCD_DATA = led_lower_nibble(PORT_LCD_DATA, *CTX);

        P_LCD_EN_PORT |= _BV(P_LCD_EN_PPIN); // LCD_E = 1 : LCD input Enable
        _delay_us(1);
        P_LCD_EN_PORT &= ~_BV(P_LCD_EN_PPIN); // LCD_E = 0 : LCD input Enable

        COROUTINE_DELAY(2);

        COROUTINE_END_CALL();
    }
}


COROUTINE(lcd_init, uint8_t)
{
    COROUTINE_LOOP()
    {
        // Serial.println("LCD Init");

        // Задержка включения питания LCD всегда > 15ms
        COROUTINE_DELAY(20);

        // Инициализация LCD 4х битным интерфейсом
        *CTX = 0x02;
        COROUTINE_CALL(lcd_command, CTX);
        // 2 строки, матрица 5*7 в 4-х битном режиме
        *CTX = 0x28;
        COROUTINE_CALL(lcd_command, CTX);
        // Выключить курсор на дисплее
        *CTX = 0x0c;
        COROUTINE_CALL(lcd_command, CTX);

        lcd_create_char(3, ch_index_3);
        lcd_create_char(7, ch_upgradus);
        lcd_create_char(6, ch_muldot);

        // Инкрементировать курсор (сдвинуть курсор вправо)
        *CTX = 0x06;
        COROUTINE_CALL(lcd_command, CTX);
        // Очистить экран дисплея
        *CTX = LCD_CLEARDISPLAY;
        COROUTINE_CALL(lcd_command, CTX);

        COROUTINE_DELAY(2);

        COROUTINE_END_CALL();
    }
}


// Отправить строку в LCD
COROUTINE(lcd_string, LCD_driver::LCDApiContext::ContextLcdString)
{
    COROUTINE_LOOP()
    {
        // Serial.print("LCD String: ");
        // Serial.println(CTX->str);

        // Отправить каждый символ строки пока не встретится 0 (конец строки)
        for (CTX->i = 0; CTX->str[CTX->i] != 0 && CTX->i < 16; ++CTX->i) {
            CTX->c = CTX->str[CTX->i];
            COROUTINE_CALL(lcd_char, &CTX->c);
        }
        COROUTINE_END_CALL();
    }
}

/*
// Отправить строку в LCD
COROUTINE(lcd_string_pgm, LCD_driver::LCDApiContext::ContextLcdStringPrgmem)
{
    COROUTINE_LOOP()
    {
        // Отправить каждый символ строки пока не встретится 0 (конец строки)
        for (CTX->i = 0; CTX->i < 16; ++CTX->i) {
            CTX->c = pgm_read_byte(CTX->str + CTX->i);
            if (CTX->c == 0) {
                break;
            }
            COROUTINE_CALL(lcd_char, &CTX->c);
        }
        COROUTINE_END_CALL();
    }
}
*/

/*
COROUTINE(lcd_string_restricted, LCD_driver::LCDApiContext::ContextLcdStringRestricted)
{
    COROUTINE_LOOP()
    {
        // Отправить каждый символ строки пока не встретится 0 (конец строки)
        for (CTX->i = 0; CTX->str[CTX->i] != 0 && CTX->i < CTX->len; CTX->i++) {
                    CTX->c = CTX->str[CTX->i];
            COROUTINE_CALL(lcd_char, &CTX->c);
        }
        COROUTINE_END_CALL();
    }
}
*/
/*
// Отправить строку в LCD в позицию X Y
COROUTINE(lcd_string_xy, LCD_driver::LCDApiContext::ContextLcdStringXY)
{
    COROUTINE_LOOP()
    {
        if (CTX->pos < 16) {
            if (CTX->row == 0) {
                // Вывод в первую строку, с требуемой позицией < 16
                                CTX->lcd_string_ctx.c = static_cast<uint8_t>((CTX->pos & 0x0F) | 0x80);
                COROUTINE_CALL(lcd_command, &CTX->lcd_string_ctx.c);
            }
            else if (CTX->row == 1) {
                // Вывод во вторую строку, с требуемой позицией < 16
                                CTX->lcd_string_ctx.c = static_cast<uint8_t>((CTX->pos & 0x0F) | 0xC0);
                COROUTINE_CALL(lcd_command, &CTX->lcd_string_ctx.c);
            }
            // Отправка строки в LCD
            CTX->lcd_string_ctx.len = 16 - CTX->pos;
            COROUTINE_CALL(lcd_string_restricted, &CTX->lcd_string_ctx);
        }
        COROUTINE_END_CALL();
    }
}
*/

COROUTINE(lcd_char_xy, LCD_driver::LCDApiContext::ContextLcdCharXY)
{
    COROUTINE_LOOP()
    {
        // Serial.print("LCD CharXY row: ");
        // Serial.print(CTX->row);
        // Serial.print(" pos: ");
        // Serial.print(CTX->pos);
        // Serial.print(" c: ");
        // Serial.println(CTX->c);

        if (CTX->pos < 16) {
            if (CTX->row == 0) {
                // Вывод в первую строку, с требуемой позицией < 16
                CTX->cmd = static_cast<uint8_t>((CTX->pos & 0x0F) | 0x80);
                COROUTINE_CALL(lcd_command, &CTX->cmd);
            }
            else if (CTX->row == 1) {
                // Вывод во вторую строку, с требуемой позицией < 16
                CTX->cmd = static_cast<uint8_t>((CTX->pos & 0x0F) | 0xC0);
                COROUTINE_CALL(lcd_command, &CTX->cmd);
            }
            // Отправка строки в LCD
            COROUTINE_CALL(lcd_char, &CTX->c);
        }
        COROUTINE_END_CALL();
    }
}


// Очистить дисплей
COROUTINE(lcd_clear, uint8_t)
{
    COROUTINE_LOOP()
    {
        // Serial.print("LCD Clear");

        *CTX = LCD_CLEARDISPLAY;
        COROUTINE_CALL(lcd_command, CTX);

        COROUTINE_DELAY(2);

        // Перевести курсор в начальную позицию
        *CTX = LCD_SETDDRAMADDR;
        COROUTINE_CALL(lcd_command, CTX);

        COROUTINE_END_CALL();
    }
}


} // namespace LCD_driver
