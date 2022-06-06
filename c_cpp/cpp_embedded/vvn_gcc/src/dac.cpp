/*
 * dac.cpp
 *
 * Created: 11.03.2022 5:48:38
 *  Author: artem
 */

#include "../arduino/Arduino.h"
#include "../arduino/SPI.h"
#include "vvn8_pins.h"

#define dac_delay() __asm__ __volatile__("nop")
	


void init_dac()
{
    P_DAC__CS_PORT |= _BV(P_DAC__CS_PPIN); // DAC_CS = 1 : DAC disable
    P_DAC__SHDN_PORT &= ~_BV(P_DAC__SHDN_PPIN); // Сброс выходного значения ЦАП
    dac_delay();
    P_DAC__SHDN_PORT |= _BV(P_DAC__SHDN_PPIN);
}

inline uint16_t prepare_dac_data_proteus(uint16_t value) {
	// bit 15: always 0 (1 means "ignore this command")
	// bit 14: buffer VREF?
	// bit 13: gain bit; 0 for 1x gain, 1 for 2x (thus we NOT the wariable)
	// bit 12: shutdown bit. 1 for active operation
	// bits 11 through 0: data 
	return (
	    // _BV(14) /* <- Buffer VREF */ 
		// |
		// _BV(13) /* <- 1 = not gain 2x */ 
		// | 
		_BV(12)  /* 1 for active operation */ 
		|
		(value & 0xFFF)
	);
}

inline void set_sck_active() {
    P_DAC_SCK__LCD_D4_PORT &= ~_BV(P_DAC_SCK__LCD_D4_PPIN); /* SCK = 0 */ // ACTIVE SCK IS LOW FOR SIMULATION
    // P_DAC_SCK__LCD_D4_PORT |= _BV(P_DAC_SCK__LCD_D4_PPIN); /* SCK = 0 */ // CLOCK SCK IS HIGH FOR SIMULATION
}

inline void set_sck_clock() {
    // P_DAC_SCK__LCD_D4_PORT &= ~_BV(P_DAC_SCK__LCD_D4_PPIN); /* SCK = 0 */ // ACTIVE SCK IS LOW FOR SIMULATION
    P_DAC_SCK__LCD_D4_PORT |= _BV(P_DAC_SCK__LCD_D4_PPIN); /* SCK = 0 */ // CLOCK SCK IS HIGH FOR SIMULATION
}


void DAC_WRITE_BIT(uint8_t value, const uint8_t bit_number)
{
    if ((value) & _BV(bit_number)) {
        P_DAC_SDI__LCD_D5_PORT |= _BV(P_DAC_SDI__LCD_D5_PPIN); /* write 1 */
    }
    else {
        P_DAC_SDI__LCD_D5_PORT &= ~_BV(P_DAC_SDI__LCD_D5_PPIN); /* write 0 */
    }
    dac_delay();
    set_sck_clock();
    dac_delay();
    set_sck_active();
    dac_delay();
}


// Запись 12 битов в ЦАП: min 0, max: 0x3FF
void dac_write(uint16_t word12)
{
    word12 = prepare_dac_data_proteus(word12);
    P_DAC__LDAC__LCD_D6_PORT |= _BV(P_DAC__LDAC__LCD_D6_PPIN); // DAC_LDAC = 1 : DAC latch high 
    dac_delay();
    P_DAC__CS_PORT &= ~_BV(P_DAC__CS_PPIN); // DAC_CS = 0 : DAC interface enable
    set_sck_active();

    {
        uint8_t bit_high = static_cast<uint8_t>((word12 & 0xFF00) >> 8);
        DAC_WRITE_BIT(bit_high, 15 - 8);
        DAC_WRITE_BIT(bit_high, 14 - 8);
        DAC_WRITE_BIT(bit_high, 13 - 8);
        DAC_WRITE_BIT(bit_high, 12 - 8);
        DAC_WRITE_BIT(bit_high, 11 - 8);
        DAC_WRITE_BIT(bit_high, 10 - 8);
        DAC_WRITE_BIT(bit_high, 9 - 8);
        DAC_WRITE_BIT(bit_high, 8 - 8);
    }
    {
        uint8_t bit_low = static_cast<uint8_t>(word12 & 0xFF);
        DAC_WRITE_BIT(bit_low, 7);
        DAC_WRITE_BIT(bit_low, 6);
        DAC_WRITE_BIT(bit_low, 5);
        DAC_WRITE_BIT(bit_low, 4);
        DAC_WRITE_BIT(bit_low, 3);
        DAC_WRITE_BIT(bit_low, 2);
        DAC_WRITE_BIT(bit_low, 1);
        DAC_WRITE_BIT(bit_low, 0);
    }
    dac_delay();

    P_DAC__LDAC__LCD_D6_PORT &= ~_BV(P_DAC__LDAC__LCD_D6_PPIN); // DAC_LDAC = 0 : DAC latch low
    dac_delay();
    P_DAC__LDAC__LCD_D6_PORT |= _BV(P_DAC__LDAC__LCD_D6_PPIN); // DAC_LDAC = 1 : DAC latch high

    dac_delay();
    P_DAC__CS_PORT |= _BV(P_DAC__CS_PPIN); // DAC_CS = 1 : DAC interface disable

    // Clear SDI pin
    // P_DAC_SDI__LCD_D5_PORT &= ~_BV(P_DAC_SDI__LCD_D5_PPIN);
}
