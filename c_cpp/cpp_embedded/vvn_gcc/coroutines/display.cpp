/*
 * display.cpp
 *
 * Created: 10.03.2022 6:43:09
 *  Author: artem
 */

#include "../arduino/Arduino.h"
#include "../coroutine/Coroutine.h"
#include "../freertos/heap.h"
#include "../src/lcd/lcd.h"
#include "../src/rtc.h"
#include "../src/state/hmi_state.h"
#include "display_context.h"
#include "../src/state/hmi_state.h"
#include "../src/state/state_utils.h"
#include "../coroutine/ctx.h"

using namespace coroutines;


#define ALPH_OFFSET (0 * 32)


COROUTINE(task_display, void)
{
    COROUTINE_LOOP()
    {
	    // Serial.println("DISPLAY INIT");
        COROUTINE_CALL(LCD_driver::lcd_init, &displayContext.ctx.c); /* Initialization of LCD*/
        COROUTINE_CALL(LCD_driver::lcd_clear, &displayContext.ctx.c);

        while (1) {
            if (!displayContext.valid) {
	            // Serial.println("DISPLAY INVALIDATE");

                for (displayContext.i = 0; displayContext.i < TEXT_BUFFER_LENGTH; ++displayContext.i) {
                    if (state::state_output_current.row0[displayContext.i] != state::state_output_previous.row0[displayContext.i]) {

                        displayContext.ctx.charXY.set(0, displayContext.i, state::state_output_current.row0[displayContext.i]);
                        COROUTINE_CALL(LCD_driver::lcd_char_xy, &displayContext.ctx.charXY);

                        // Запись факта обновления состояния отображаемого символа в контекст предыдущего состояния
                        state::state_output_previous.row0[displayContext.i] = state::state_output_current.row0[displayContext.i];
                    }
                }
                for (displayContext.i = 0; displayContext.i < TEXT_BUFFER_LENGTH; ++displayContext.i) {
                    if (state::state_output_current.row1[displayContext.i] != state::state_output_previous.row1[displayContext.i]) {

                        displayContext.ctx.charXY.set(1, displayContext.i, state::state_output_current.row0[displayContext.i]);
                        COROUTINE_CALL(LCD_driver::lcd_char_xy, &displayContext.ctx.charXY);

                        // Запись факта обновления состояния отображаемого символа в контекст предыдущего состояния
                        state::state_output_previous.row1[displayContext.i] = state::state_output_current.row1[displayContext.i];
                    }
                }
                displayContext.valid      = true;
                displayContext.valid_full = true;
                COROUTINE_YIELD();
            }
            else if (!displayContext.valid_full) {
	            // Serial.println("DISPLAY INVALIDATE FULL");

                COROUTINE_CALL(LCD_driver::lcd_clear, &displayContext.ctx.c);

                displayContext.ctx.lcdString.set(state::state_output_current.row0);

                COROUTINE_CALL(LCD_driver::lcd_string,  &displayContext.ctx.lcdString); /* Write string on 1st line of LCD*/

                displayContext.ctx.c = LCD_COMMAND_GO_TO_2_LINE;
                COROUTINE_CALL(LCD_driver::lcd_command, &displayContext.ctx.c); /* Go to 2nd line*/

                displayContext.ctx.lcdString.set(state::state_output_current.row1);

                COROUTINE_CALL(LCD_driver::lcd_string, &displayContext.ctx.lcdString); /* Write string on 1st line of LCD*/

                displayContext.valid      = true;
                displayContext.valid_full = true;
            }
            else {
	            // Serial.println("DISPLAY SLEEP");

                COROUTINE_DELAY_MAIN(1000 / 30);
            }
        }
    }
}
