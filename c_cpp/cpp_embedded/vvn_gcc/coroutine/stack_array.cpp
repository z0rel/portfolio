/*
 * stack_array.cpp
 *
 * Created: 18.03.2022 13:02:02
 *  Author: artem
 */

#include "stack_array.h"
#include <avr/pgmspace.h>

namespace multitasking {


namespace internal_multitasking {

static const char _msg_stack_is_full[] PROGMEM             = "===== STACK: stack is full. size=";
static const char _msg_cant_pop_item_from_stack[] PROGMEM  = "===== STACK: can't pop item from stack: stack is empty. size=";
static const char _msg_cant_peek_item_from_stack[] PROGMEM = "===== STACK: can't peek item from stack: stack is empty. size=";
static const char _msg_top_stack[] PROGMEM = ", stack top=";


static void serial_print(const __FlashStringHelper *ifsh, uint8_t size, uint8_t stack_top) {
    Serial.print(ifsh);
    Serial.print(size);
    const __FlashStringHelper *ifsh1 = reinterpret_cast<const __FlashStringHelper *>(_msg_top_stack);
    Serial.print(ifsh1);
    Serial.print(stack_top);
    Serial.println();
}

void PrintMessage::stack_is_full(uint8_t size, uint8_t stack_top) 
{
    serial_print(reinterpret_cast<const __FlashStringHelper *>(_msg_stack_is_full), size, stack_top);
}

void PrintMessage::cant_pop_item_from_stack(uint8_t size, uint8_t stack_top)
{
    serial_print(reinterpret_cast<const __FlashStringHelper *>(_msg_cant_pop_item_from_stack), size, stack_top);
}

void PrintMessage::cant_peek_item_from_stack(uint8_t size, uint8_t stack_top)
{
    serial_print(reinterpret_cast<const __FlashStringHelper *>(_msg_cant_peek_item_from_stack), size, stack_top);
}


} // namespace internal_multitasking
}
