/*
 * init_gpio.cpp
 *
 * Created: 01.03.2022 22:29:20
 *  Author: artem
 */ 

#include <stdint.h>
#include <avr/pgmspace.h>
#include "common_items.h"
#include "vvn8_pins.h"
#include "../arduino/Arduino.h"


const PROGMEM uint8_t PINS_OUTPUT_LOW[] = {
    P_A0_D0,       // External RAM interface 
    P_A1_D1,       // External RAM interface 
    P_A2_D2,       // External RAM interface 
    P_A3_D7,       // External RAM interface 
    P_A4_D6,       // External RAM interface 
    P_A5_D5,       // External RAM interface 
    P_A6_D4,       // External RAM interface 
    P_A7_D3,       // External RAM interface 
    P__WR_MEM,     // External RAM interface 
    P__RD_MEM,     // External RAM interface 
    P_ALE_MEM,     // External RAM interface 
    P_A8,          // External RAM interface 
    P_A9,          // External RAM interface 
    P_A10,         // External RAM interface 
    P_A11,         // External RAM interface 
    P_A12,         // External RAM interface 
    P_A13,         // External RAM interface 
    P_A14,         // External RAM interface 
    P__CE_A15_MEM, // External RAM interface, должен быть 0

    P_52,              
    P_OUT_LOW_SET,       
    P_OUT_HIGH_SET,    
    P_OUT_LOW_SET,     
    P_OUT_HIGH_SET,    
    P_DEN,             
    P_SOUND_PE4,       
    P_LCD_RW,          
    P_LCD_RS,          


    P_LCD_D7,          
    P_DAC_SDI__LCD_D5, 
    P_DAC_SCK__LCD_D4, 
    P_LCD_EN,           
};


const PROGMEM uint8_t PINS_OUTPUT_HIGH[] = {
    P_MOSI,            
    P_LED_BRIGHT,      
    P_LED_LOW_SET,     
    P_LED_HIGH_SET,    
    P_DAC__CS,         
    P_DAC__SHDN,       
    P_DAC__LDAC__LCD_D6,

};


const PROGMEM uint8_t PINS_INP[] = { 
    P_BTN_TIRE,
    P_BTN_F,
    P_BTN_ENT,
    P_BTN_ESC,
    P_BTN_DOWN,
	P_PNT6_PE7
};


void init_gpio() {
	unsigned int i;

	for (i = 0; i < SIZEOF_ARRAY(PINS_OUTPUT_LOW); ++i) {
	    uint8_t pin = pgm_read_byte(&PINS_OUTPUT_LOW[i]);
		pinMode(pin, OUTPUT);
		digitalWrite(pin, 0);
	}
	for (i = 0; i < SIZEOF_ARRAY(PINS_OUTPUT_HIGH); ++i) {
	    uint8_t pin = pgm_read_byte(&PINS_OUTPUT_HIGH[i]);
		pinMode(pin, OUTPUT);
		digitalWrite(pin, 1);
	}
	for (i = 0; i < SIZEOF_ARRAY(PINS_INP); ++i) {
	    uint8_t pin = pgm_read_byte(&PINS_INP[i]);
		pinMode(pin, INPUT_PULLUP);
	}
}
