/*
 * lcd.h
 *
 * Created: 08.03.2022 15:30:22
 *  Author: artem
 */


#ifndef LCD_H_
#define LCD_H_

#include <stdint.h>

#include "../../coroutine/Coroutine.h"

#include "lcd_api_context.h"
#include "lcd_commands.h"


namespace LCD_driver {

/// Инициализация дисплея
EXTERN_COROUTINE(lcd_init, uint8_t);

/// Отправка символа в LCD
EXTERN_COROUTINE(lcd_char, uint8_t);

/// Отправка команды в LCD
EXTERN_COROUTINE(lcd_command, uint8_t);

/// Отправить строку в LCD
EXTERN_COROUTINE(lcd_string, LCD_driver::LCDApiContext::ContextLcdString);
// EXTERN_COROUTINE(lcd_string_pgm, LCD_driver::LCDApiContext::ContextLcdStringPrgmem);

/// Отправить строку в LCD в позицию X Y
// EXTERN_COROUTINE(lcd_string_xy, LCD_driver::LCDApiContext::ContextLcdStringXY);

EXTERN_COROUTINE(lcd_char_xy, LCD_driver::LCDApiContext::ContextLcdCharXY);

/// Очистить дисплей
EXTERN_COROUTINE(lcd_clear, uint8_t);

}  // namespace LCD_driver


#endif /* LCD_H_ */
