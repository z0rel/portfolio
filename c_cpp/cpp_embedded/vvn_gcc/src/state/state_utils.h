/*
 * state_utils.h
 *
 * Created: 16.03.2022 19:14:17
 *  Author: artem
 */


#ifndef STATE_UTILS_H_
#define STATE_UTILS_H_

#include <stdint.h>
#include <avr/pgmspace.h>
#include "hmi_state.h"
#include "../../coroutines/display_context.h"

namespace state {



#define TEXT_BUFFER_LENGTH (sizeof(state::state_output_current.row0) - 1)


#define TEXT_BUFFER_LENGTH_SNPRINTF (sizeof(state::state_output_current.row0))

bool waitDisplayVaild();
bool waitDisplayVaildFull();


#define WAIT_INVALIDATE_DISPLAY() \
    coroutines::displayContext.invalidate();  \
    COROUTINE_AWAIT_CB(&state::waitDisplayVaild)


#define WAIT_INVALIDATE_DISPLAY_FUN(fun) \
    (fun);                               \
    WAIT_INVALIDATE_DISPLAY()


#define WAIT_INVALIDATE_DISPLAY_FULL() \
    coroutines::displayContext.invalidateFull();   \
    COROUTINE_AWAIT_CB(&state::waitDisplayVaildFull)


#define WAIT_INVALIDATE_DISPLAY_FULL_FUN(fun) \
    (fun);                                    \
    WAIT_INVALIDATE_DISPLAY_FULL()


void set_menu_state(HMI_Menu state);
void set_logo_state();
void set_row_text(char *dst, const char *src PROGMEM, uint8_t length);
void set_row_text_shift(char *dst, const char *src PROGMEM, uint8_t length, uint8_t shift);
void step_state_logo(uint8_t shift);


} // namespace state


#endif /* STATE_UTILS_H_ */
