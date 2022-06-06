/*
 * vvn8_pins.h
 *
 * Created: 01.03.2022 5:58:20
 *  Author: artem
 */ 


#ifndef VVN8_PINS_H_
#define VVN8_PINS_H_


#include "../arduino/pins_arduino.h"

#define P_A0_D0                   PIN_PA0
#define P_A1_D1                   PIN_PA1
#define P_A2_D2                   PIN_PA2
#define P_A3_D7                   PIN_PA3
#define P_A4_D6                   PIN_PA4
#define P_A5_D5                   PIN_PA5
#define P_A6_D4                   PIN_PA6
#define P_A7_D3                   PIN_PA7

#define P__WR_MEM                 PIN_PG0
#define P__RD_MEM                 PIN_PG1
#define P_ALE_MEM                 PIN_PG2

#define P_A8                      PIN_PC0
#define P_A9                      PIN_PC1
#define P_A10                     PIN_PC2
#define P_A11                     PIN_PC3
#define P_A12                     PIN_PC4
#define P_A13                     PIN_PC5
#define P_A14                     PIN_PC6
#define P__CE_A15_MEM             PIN_PC7


#define P_LED_LOW_SET             PIN_PB0
#define P_52                      PIN_PB1
#define P_MOSI                    PIN_PB2
#define P_BTN_TIRE                PIN_PB3
#define P_BTN_F                   PIN_PB4
#define P_BTN_ENT                 PIN_PB5
#define P_BTN_ESC                 PIN_PB6
#define P_BTN_DOWN                PIN_PB7

#define P_OUT_LOW_SET             PIN_PD0
#define P_OUT_HIGH_SET            PIN_PD1
#define P_RXD                     PIN_PD2
#define P_TXD                     PIN_PD3
#define P_DEN                     PIN_PD4

#define P_DEN_PORT                PORTD
#define P_DEN_PIN                 4

#define P_DAC__CS                 PIN_PD5
#define P_DAC__SHDN               PIN_PD7

#define P_60                      PIN_PE0
#define P_59                      PIN_PE1
#define P_LCD_RW                  PIN_PE2
#define P_LCD_RS                  PIN_PE3
#define P_SOUND_PE4               PIN_PE4
#define P_LED_BRIGHT              PIN_PE5
#define P_LED_HIGH_SET            PIN_PE6
#define P_PNT6_PE7                PIN_PE7

#define P_LCD_D7                  PIN_PF0
#define P_DAC__LDAC__LCD_D6       PIN_PF1
#define P_DAC_SDI__LCD_D5         PIN_PF2
#define P_DAC_SCK__LCD_D4         PIN_PF3
#define P_LCD_EN                  PIN_PF4
#define P_P_ADC5                  PIN_PF5
#define P_T_ADC6                  PIN_PF6
#define P_VVN_ADC7                PIN_PF7

#define P_A0_D0_PORT              PORTA
#define P_A0_D0_PPIN              0
#define P_A1_D1_PORT              PORTA
#define P_A1_D1_PPIN              1
#define P_A2_D2_PORT              PORTA
#define P_A2_D2_PPIN              2
#define P_A3_D7_PORT              PORTA
#define P_A3_D7_PPIN              3
#define P_A4_D6_PORT              PORTA
#define P_A4_D6_PPIN              4
#define P_A5_D5_PORT              PORTA
#define P_A5_D5_PPIN              5
#define P_A6_D4_PORT              PORTA
#define P_A6_D4_PPIN              6
#define P_A7_D3_PORT              PORTA
#define P_A7_D3_PPIN              7

#define P_WR_MEM_PORT             PORTG
#define P_WR_MEM_PPIN             0
#define P_RD_MEM_PORT             PORTG
#define P_RD_MEM_PPIN             1
#define P_ALE_MEM_PORT            PORTG
#define P_ALE_MEM_PPIN            2

#define P_A8_PORT                 PORTC
#define P_A8_PPIN                 0
#define P_A9_PORT                 PORTC
#define P_A9_PPIN                 1
#define P_A10_PORT                PORTC
#define P_A10_PPIN                2
#define P_A11_PORT                PORTC
#define P_A11_PPIN                3
#define P_A12_PORT                PORTC
#define P_A12_PPIN                4
#define P_A13_PORT                PORTC
#define P_A13_PPIN                5
#define P_A14_PORT                PORTC
#define P_A14_PPIN                6
#define P_CE_A15_MEM_PORT         PORTC
#define P_CE_A15_MEM_PPIN         7

#define P_LED_LOW_SET_PORT        PORTB 
#define P_LED_LOW_SET_PPIN        0
#define P_52_PORT                 PORTB
#define P_52_PPIN                 1
#define P_MOSI_PORT               PORTB
#define P_MOSI_PPIN               2
#define P_BTN_TIRE_PORT           PORTB
#define P_BTN_TIRE_PPIN           3
#define P_BTN_F_PORT              PORTB
#define P_BTN_F_PPIN              4
#define P_BTN_ENT_PORT            PORTB
#define P_BTN_ENT_PPIN            5
#define P_BTN_ESC_PORT            PORTB
#define P_BTN_ESC_PPIN            6
#define P_BTN_DOWN_PORT           PORTB
#define P_BTN_DOWN_PPIN           7

#define P_OUT_LOW_SET_PORT        PORTD
#define P_OUT_LOW_SET_PPIN        0
#define P_OUT_HIGH_SET_PORT       PORTD
#define P_OUT_HIGH_SET_PPIN       1
#define P_RXD_PORT                PORTD
#define P_RXD_PPIN                2
#define P_TXD_PORT                PORTD
#define P_TXD_PPIN                3
#define P_DEN_PORT                PORTD
#define P_DEN_PPIN                4
#define P_DAC__CS_PORT            PORTD
#define P_DAC__CS_PPIN            5
#define P_DAC__SHDN_PORT          PORTD
#define P_DAC__SHDN_PPIN          7

#define P_60_PORT                 PORTE
#define P_60_PPIN                 0
#define P_59_PORT                 PORTE
#define P_59_PPIN                 1
#define P_LCD_RW_PORT             PORTE  // Не используется, можно подтянуть к 0
#define P_LCD_RW_PPIN             2
#define P_LCD_RS_PORT             PORTE
#define P_LCD_RS_PPIN             3
#define P_SOUND_PE4_PORT          PORTE
#define P_SOUND_PE4_PPIN          4
#define P_LED_BRIGHT_PORT         PORTE
#define P_LED_BRIGHT_PPIN         5
#define P_LED_HIGH_SET_PORT       PORTE
#define P_LED_HIGH_SET_PPIN       6
#define P_PNT6_PE7_PORT           PORTE
#define P_PNT6_PE7_PPIN           7

#define P_LCD_D7_PORT             PORTF
#define P_LCD_D7_PPIN             0
#define P_DAC__LDAC__LCD_D6_PORT  PORTF
#define P_DAC__LDAC__LCD_D6_PPIN  1
#define P_DAC_SDI__LCD_D5_PORT    PORTF
#define P_DAC_SDI__LCD_D5_PPIN    2
#define P_DAC_SCK__LCD_D4_PORT    PORTF
#define P_DAC_SCK__LCD_D4_PPIN    3
#define P_LCD_EN_PORT             PORTF
#define P_LCD_EN_PPIN             4
#define P_P_ADC5_PORT             PORTF
#define P_P_ADC5_PPIN             5
#define P_T_ADC6_PORT             PORTF
#define P_T_ADC6_PPIN             6
#define P_VVN_ADC7_PORT           PORTF
#define P_VVN_ADC7_PPIN           7



#endif /* VVN8_PINS_H_ */