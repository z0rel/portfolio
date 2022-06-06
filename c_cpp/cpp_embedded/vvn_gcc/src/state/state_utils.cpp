/*
 * state_utils.cpp
 *
 * Created: 16.03.2022 19:39:03
 *  Author: artem
 */ 

#include "state_utils.h"


namespace state {


void set_row_text(char *dst, const char *src PROGMEM, uint8_t length)
{
    memcpy_P(dst, src, length);
    for (uint8_t i = length; i < TEXT_BUFFER_LENGTH; ++i) {
        dst[i] = ' ';
    }
}


void set_row_text_shift(char *dst, const char *src PROGMEM, uint8_t length, uint8_t shift)
{
    if (shift > TEXT_BUFFER_LENGTH) {
        shift = TEXT_BUFFER_LENGTH;
    }
    for (uint8_t i = 0; i < shift; ++i) {
        dst[i] = ' ';
    }

    if (shift < TEXT_BUFFER_LENGTH) {
        uint8_t copy_length = length;
        uint8_t slen        = shift + length;
        if (slen > TEXT_BUFFER_LENGTH) {
            copy_length -= (slen - TEXT_BUFFER_LENGTH);
            slen = TEXT_BUFFER_LENGTH;
        }
        memcpy_P(dst + shift, src, copy_length);

        for (uint8_t i = slen; i < TEXT_BUFFER_LENGTH; ++i) {
            dst[i] = ' ';
        }
    }
}



void set_menu_state(HMI_Menu state)
{
    state::state_output_previous.hmi_menu_item = state::state_output_current.hmi_menu_item;
    state::state_output_current.hmi_menu_item  = state;
}




}
